#include "chat/client/widget/reset_dialog.h"

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/widget/ui_reset_dialog.h"

ResetDialog::ResetDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ResetDialog) {
    ui->setupUi(this);

    // 连接输入框的错误信号
    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkUserValid(ui->user_edit->text()); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkEmailValid(ui->email_edit->text()); });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkVerifyValid(ui->verify_edit->text()); });
    connect(ui->password_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkPasswordValid(ui->password_edit->text()); });

    // 连接 http 请求完成信号
    initHttpHandlers();
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reset_mod_finish, this, &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog() { delete ui; }

void ResetDialog::on_verify_btn_clicked() {
    auto email = ui->email_edit->text();
    if (ui->err_tip->checkEmailValid(email)) {
        // 发送验证码
        QJsonObject root;
        root["email"] = email;
        HttpMgr::GetInstance().PostHttpRequest(kModule, ReqId::kGetVerifyCode, root);
    }
}

void ResetDialog::on_sure_btn_clicked() {
    if (ui->err_tip->checkUserValid(ui->user_edit->text()) &&
        ui->err_tip->checkEmailValid(ui->email_edit->text()) &&
        ui->err_tip->checkVerifyValid(ui->verify_edit->text()) &&
        ui->err_tip->checkPasswordValid(ui->password_edit->text())) {
        QJsonObject root;
        root["user"] = ui->user_edit->text();
        root["email"] = ui->email_edit->text();
        root["password"] = xorString(ui->password_edit->text());
        root["verify_code"] = ui->verify_edit->text();
        HttpMgr::GetInstance().PostHttpRequest(kModule, ReqId::kResetPassword, root);
    }
}

void ResetDialog::on_return_btn_clicked() { emit switchLogin(); }

void ResetDialog::slot_reset_mod_finish(ReqId req_id, const QString& res, ErrorCode err) {
    auto json_doc = ui->err_tip->checkHttpResponse(res, err);
    if (json_doc) {
        handlers_[req_id](json_doc->object());
    }
}

void ResetDialog::initHttpHandlers() {
    handlers_.emplace(ReqId::kGetVerifyCode, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            ui->err_tip->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        ui->err_tip->showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "email is" << email;
    });
    handlers_.emplace(ReqId::kResetPassword, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            ui->err_tip->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        ui->err_tip->showTip(tr("重置成功, 点击返回登录"), true);
        qDebug() << "email is" << email;
    });
}
