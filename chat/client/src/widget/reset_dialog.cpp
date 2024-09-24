#include "chat/client/widget/reset_dialog.h"

#include <QJsonDocument>

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/url_mgr.h"
#include "chat/client/widget/ui_reset_dialog.h"

ResetDialog::ResetDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ResetDialog) {
    ui->setupUi(this);

    // 连接输入框的错误信号
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkUserValid(ui->user_edit->text())); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkEmailValid(ui->email_edit->text())); });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkVerifyValid(ui->verify_edit->text())); });
    connect(ui->password_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkPasswordValid(ui->password_edit->text())); });

    // 连接 http 请求完成信号
    initHttpHandlers();
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reset_mod_finish, this, &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog() { delete ui; }

void ResetDialog::on_verify_btn_clicked() {
    auto email = ui->email_edit->text();
    auto check = input_check_.checkEmailValid(email);
    if (check) {
        showTip(tr(check->data()), false);
        return;
    }

    // 发送验证码
    QJsonObject root;
    root["email"] = email;
    HttpMgr::GetInstance().PostHttpRequest(QUrl(UrlMgr::GetInstance().GateUrlPrefix() + "/get_verifycode"), root,
                                           ReqId::kGetVerifyCode, Module::kResetMod);
}

void ResetDialog::on_sure_btn_clicked() {
    if (input_check_.checkUserValid(ui->user_edit->text()) ||
        input_check_.checkEmailValid(ui->email_edit->text()) ||
        input_check_.checkVerifyValid(ui->verify_edit->text()) ||
        input_check_.checkPasswordValid(ui->password_edit->text())) {
        showErrTip(input_check_.getLastError());
        return;
    }

    QJsonObject root;
    root["user"] = ui->user_edit->text();
    root["email"] = ui->email_edit->text();
    root["password"] = xorString(ui->password_edit->text());
    root["verify_code"] = ui->verify_edit->text();
    HttpMgr::GetInstance().PostHttpRequest(QUrl(UrlMgr::GetInstance().GateUrlPrefix() + "/reset_pwd"), root,
                                           ReqId::kRestPassword, Module::kResetMod);
}

void ResetDialog::on_return_btn_clicked() { emit switchLogin(); }

void ResetDialog::slot_reset_mod_finish(ReqId req_id, const QString& res, ErrorCode err) {
    if (err != ErrorCode::kSuccess) {
        showTip(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(res.toUtf8());
    if (json_doc.isEmpty()) {
        showTip(tr("json解析失败"), false);
        return;
    }
    if (!json_doc.isObject()) {
        showTip(tr("json解析失败"), false);
        return;
    }

    handlers_[req_id](json_doc.object());
    return;
}

void ResetDialog::showTip(const QString& str, bool normal) {
    ui->err_tip->setText(str);
    if (normal) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    repolish(ui->err_tip);
}

void ResetDialog::showErrTip(std::optional<std::string_view> str) {
    if (str) {
        showTip(tr(str->data()), false);
    } else {
        ui->err_tip->clear();
    }
}

void ResetDialog::initHttpHandlers() {
    handlers_.emplace(ReqId::kGetVerifyCode, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            this->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        this->showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "email is" << email;
    });
    handlers_.emplace(ReqId::kRestPassword, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            this->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        this->showTip(tr("重置成功, 点击返回登录"), true);
        qDebug() << "email is" << email;
    });
}
