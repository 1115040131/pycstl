#pragma once

#include <functional>
#include <map>

#include <QDialog>
#include <QJsonObject>

#include "chat/client/define.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

private:
    // 初始化头像
    void initHead();

    // 设置按钮是否可用
    void enableButton(bool enable);

    // 初始化 http 回复处理
    void initHttpHandlers();

signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(const ServerInfo& server_info);

private slots:
    // 忘记密码
    void slot_forget_password();

    // 登录按钮点击
    void on_login_btn_clicked();

    // 注册按钮点击
    void on_reg_btn_clicked();

    // http 请求完成
    void slot_login_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

    // tcp 连接完成
    void slot_tcp_connect_finish(bool success);

    // 登录失败
    void slot_login_failed(ErrorCode err);

private:
    static constexpr Module kModule{Module::kLogin};

    Ui::LoginDialog* ui;
    std::map<ReqId, std::function<void(const QJsonObject&)>> handlers_;

    int uid_;
    QString token_;
};