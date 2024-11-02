#pragma once

#include <QObject>
#include <QTcpSocket>

#include "chat/client/define.h"
#include "chat/client/search_info.h"
#include "common/singleton.h"

class TcpMgr : public QObject, public pyc::Singleton<TcpMgr> {
    Q_OBJECT
    friend class pyc::Singleton<TcpMgr>;

private:
    TcpMgr();

public:
    ~TcpMgr() = default;

private:
    // 初始化 http 回复处理
    void initHttpHandlers();

    void handleMessage(ReqId req_id, uint16_t len, const QByteArray& data);

signals:
    // tcp 连接完成
    void sig_tcp_connect_finish(bool success);

    // 发送消息
    void sig_send_data(ReqId req_id, const QString& data);

    // 登录失败
    void sig_login_failed(ErrorCode err);

    // 切换到聊天界面
    void sig_switch_chatdlg();

    // 搜索用户
    void sig_user_search(const SearchInfo& search_info);

public slots:
    // 连接到聊天服务器
    void slot_connect_tcp(const ServerInfo& server_info);

private slots:
    // 发送消息
    void slot_send_data(ReqId req_id, const QString& data);

private:
    QTcpSocket socket_;
    QString host_{""};
    uint port_{0};
    QByteArray buffer_;
    bool recv_pending_{false};  // 是否解析完头部
    uint16_t message_id_{0};
    uint16_t message_len_{0};
    static constexpr qsizetype kHeadLength = sizeof(message_id_) + sizeof(message_len_);

    std::map<ReqId, std::function<void(const QByteArray&)>> handlers_;
};