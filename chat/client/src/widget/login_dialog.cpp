#include "chat/client/widget/login_dialog.h"

#include <QPainter>
#include <QPainterPath>

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/tcp_mgr.h"
#include "chat/client/widget/ui_login_dialog.h"

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);

    ui->err_tip->clear();

    // 忘记密码
    ui->forget_label->setState("normal", "hover", "hover", "selected", "selected_hover", "selected_hover");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_password);

    // 初始化头像
    initHead();

    // 连接 http 请求完成信号
    initHttpHandlers();
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);

    // 连接 tcp 连接请求
    connect(this, &LoginDialog::sig_connect_tcp, &TcpMgr::GetInstance(), &TcpMgr::slot_connect_tcp);
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_tcp_connect_finish, this, &LoginDialog::slot_tcp_connect_finish);
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::initHead() {
    QPixmap original_pixmap("chat/client/res/head_2.jpg");
    // 设置图片自动缩放
    original_pixmap =
        original_pixmap.scaled(ui->head_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个圆角图片
    QPixmap rounded_pixmap(original_pixmap.size());
    rounded_pixmap.fill(Qt::transparent);

    QPainter painter(&rounded_pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);           // 设置抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);  // 设置平滑变换

    // 设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, original_pixmap.width(), original_pixmap.height(), 10, 10);
    painter.setClipPath(path);

    // 将原始图片绘制到圆角图片上
    painter.drawPixmap(0, 0, original_pixmap);

    ui->head_label->setPixmap(rounded_pixmap);
}

void LoginDialog::enableButton(bool enable) {
    ui->login_btn->setEnabled(enable);
    ui->reg_btn->setEnabled(enable);
}

void LoginDialog::initHttpHandlers() {
    handlers_.emplace(ReqId::kLogin, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error == ErrorCode::kPasswordError) {
            ui->err_tip->showTip(tr("用户名或密码错误"), false);
            enableButton(true);
            return;
        }

        if (error != ErrorCode::kSuccess) {
            ui->err_tip->showTip(tr("参数错误"), false);
            enableButton(true);
            return;
        }

        auto email = json["email"].toString();

        ServerInfo server_info{
            .uid = json["uid"].toInt(),
            .host = json["host"].toString(),
            .port = json["port"].toString(),
            .token = json["token"].toString(),
        };
        uid_ = server_info.uid;
        token_ = server_info.token;

        qDebug() << "kLogin finish: uid:" << server_info.uid << "host:" << server_info.host
                 << "port:" << server_info.port << "token:" << server_info.token;
        emit sig_connect_tcp(server_info);
    });
}

void LoginDialog::slot_forget_password() { emit switchReset(); }

void LoginDialog::on_login_btn_clicked() {
    if (ui->err_tip->checkEmailValid(ui->email_edit->text()) &&
        ui->err_tip->checkPasswordValid(ui->password_edit->text())) {
        enableButton(false);
        QJsonObject root;
        root["email"] = ui->email_edit->text();
        root["password"] = xorString(ui->password_edit->text());
        HttpMgr::GetInstance().PostHttpRequest(kModule, ReqId::kLogin, root);
    }
}

void LoginDialog::on_reg_btn_clicked() { emit switchRegister(); }

void LoginDialog::slot_login_mod_finish(ReqId req_id, const QString& res, ErrorCode err) {
    auto json_doc = ui->err_tip->checkHttpResponse(res, err);
    if (json_doc) {
        handlers_[req_id](json_doc->object());
    }
}

void LoginDialog::slot_tcp_connect_finish(bool success) {
    if (success) {
        ui->err_tip->showTip("聊天服务器连接成功, 正在登录...", true);

        QJsonObject root;
        root["uid"] = uid_;
        root["token"] = token_;

        // 发送登录信息给 chat server 进行逻辑意义上的登录
        QJsonDocument doc(root);
        emit TcpMgr::GetInstance().sig_send_data(ReqId::kChatLogin, doc.toJson(QJsonDocument::Indented));
    } else {
        ui->err_tip->showTip("网络异常", false);
        enableButton(true);
    }
}

void LoginDialog::slot_login_failed(ErrorCode err) {
    QString result = QString("登陆失败, err is %1").arg(ToString(err));
    ui->err_tip->showTip(result, false);
    enableButton(true);
}
