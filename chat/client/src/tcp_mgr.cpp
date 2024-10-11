#include "chat/client/tcp_mgr.h"

#include <QJsonDocument>
#include <QJsonObject>

TcpMgr::TcpMgr() {
    QObject::connect(&socket_, &QTcpSocket::connected, [this]() {
        qDebug() << "Connected to server!";
        // 连接成功发送消息
        emit sig_tcp_connect_finish(true);
    });

    QObject::connect(&socket_, &QTcpSocket::readyRead, [this]() {
        // 当有数据可读时, 读取所有数据到缓冲区
        buffer_.append(socket_.readAll());

        QDataStream stream(&buffer_, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_0);

        forever {
            // 解析头部
            if (!recv_pending_) {
                if (buffer_.size() < kHeadLength) {
                    return;  // 数据头长度不够
                }

                // 读取头部
                stream >> message_id_ >> message_len_;

                // 移除头部内容
                buffer_ = buffer_.mid(kHeadLength);

                qDebug() << "Message ID:" << message_id_ << ", Length:" << message_len_;
            }

            // 数据体长度不够
            if (buffer_.size() < message_len_) {
                recv_pending_ = true;
                return;
            }

            // 解析消息体
            recv_pending_ = false;
            QByteArray message_body = buffer_.mid(0, message_len_);
            qDebug() << "Receive message:" << message_body;

            buffer_ = buffer_.mid(message_len_);
            handleMessage(static_cast<ReqId>(message_id_), message_len_, message_body);
        }
    });

    // 处理错误（适用于Qt 5.15之前的版本）
    // 5.15 之后版本
    QObject::connect(&socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                     [&](QAbstractSocket::SocketError socketError) {
                         Q_UNUSED(socketError)
                         qDebug() << "Error:" << socket_.errorString();
                         emit sig_tcp_connect_finish(false);
                     });

    // 处理连接断开
    QObject::connect(&socket_, &QTcpSocket::disconnected, [this]() { qDebug() << "Disconnected from server"; });

    // 连接发送信号用来发送数据
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);

    // 注册消息
    initHttpHandlers();
}

void TcpMgr::initHttpHandlers() {
    handlers_.emplace(ReqId::kChatLoginRes, [this](const QByteArray& data) {
        QJsonDocument json_doc = QJsonDocument::fromJson(data);

        if (json_doc.isNull()) {
            qDebug() << "Failed to create QJsonDocument";
            return;
        }

        QJsonObject json = json_doc.object();

        if (!json.contains("error")) {
            qDebug() << "Login Failed, err is ErrorCode::kJsonError";
            emit sig_login_failed(ErrorCode::kJsonError);
            return;
        }

        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            qDebug() << "Login Failed, err is" << ToString(error);
            emit sig_login_failed(error);
            return;
        }

        qDebug() << "Login Success!";
        emit sig_switch_chatdlg();
    });
}

void TcpMgr::handleMessage(ReqId req_id, uint16_t, const QByteArray& data) {
    auto iter = handlers_.find(req_id);
    if (iter == handlers_.end()) {
        qDebug() << "not found id [" << ToString(req_id) << "] to handle";
        return;
    }
    iter->second(data);
}

void TcpMgr::slot_connect_tcp(const ServerInfo& server_info) {
    qDebug() << "Connecting to server...";
    host_ = server_info.host;
    port_ = server_info.port.toUInt();
    socket_.connectToHost(host_, port_);
}

void TcpMgr::slot_send_data(ReqId req_id, const QString& data) {
    uint16_t message_id = static_cast<uint16_t>(req_id);

    // 将字符串转为 UTF-8 编码的字节数组
    QByteArray data_bytes = data.toUtf8();
    uint16_t message_len = data.size();

    // 创建一个存储所有发送数据的字节数组
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    out << message_id << message_len;
    block.append(data_bytes);

    // 发送数据
    socket_.write(block);
}