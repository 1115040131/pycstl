#include "chat/client/widget/register_dialog.h"

#include <QJsonDocument>
#include <QRegularExpression>

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/url_mgr.h"
#include "chat/client/widget/ui_register_dialog.h"

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    Repolish(ui->err_tip);

    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    initHttpHandlers();

    // 连接输入框的错误信号
    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this, [this]() { checkUserValid(); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() { checkEmailValid(); });
    connect(ui->password_edit, &QLineEdit::editingFinished, this, [this]() { checkPasswordValid(); });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this]() { checkConfirmValid(); });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this]() { checkVerifyValid(); });
}

RegisterDialog::~RegisterDialog() { delete ui; }

void RegisterDialog::on_get_code_btn_clicked() {
    auto email = ui->email_edit->text();
    QRegularExpression regex(kEmailRegex.data());
    bool match = regex.match(email).hasMatch();
    if (match) {
        // 发送验证码
        QJsonObject root;
        root["email"] = email;
        HttpMgr::GetInstance().PostHttpRequest(QUrl(UrlMgr::GetInstance().GateUrlPrefix() + "/get_verifycode"),
                                               root, ReqId::kGetVarifyCode, Module::kRegisterMod);
    } else {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::on_sure_btn_clicked() {
    if (ui->user_edit->text() == "") {
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if (ui->email_edit->text() == "") {
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if (ui->password_edit->text() == "") {
        showTip(tr("密码不能为空"), false);
        return;
    }
    if (ui->confirm_edit->text() == "") {
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if (ui->confirm_edit->text() != ui->password_edit->text()) {
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }
    if (ui->verify_edit->text() == "") {
        showTip(tr("验证码不能为空"), false);
        return;
    }

    QJsonObject root;
    root["user"] = ui->user_edit->text();
    root["email"] = ui->email_edit->text();
    root["password"] = xorString(ui->password_edit->text());
    root["confirm"] = xorString(ui->confirm_edit->text());
    root["verify_code"] = ui->verify_edit->text();
    HttpMgr::GetInstance().PostHttpRequest(QUrl(UrlMgr::GetInstance().GateUrlPrefix() + "/user_register"), root,
                                           ReqId::kRegUser, Module::kRegisterMod);
}

void RegisterDialog::slot_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err) {
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

void RegisterDialog::initHttpHandlers() {
    handlers_.emplace(ReqId::kGetVarifyCode, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            this->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        this->showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "email is" << email;
    });
    handlers_.emplace(ReqId::kRegUser, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            this->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        this->showTip(tr("用户注册成功"), true);
        qDebug() << "email is" << email;
        qDebug() << "user uid is" << json["uid"].toInt();
    });
}

void RegisterDialog::showTip(const QString& str, bool normal) {
    ui->err_tip->setText(str);
    if (normal) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    Repolish(ui->err_tip);
}

void RegisterDialog::addTipErr(TipErr tip_err, QString msg) {
    tip_errs_[tip_err] = msg;
    showTip(msg, false);
}

void RegisterDialog::delTipErr(TipErr tip_err) {
    tip_errs_.erase(tip_err);
    if (tip_errs_.empty()) {
        ui->err_tip->clear();
        return;
    }
    showTip(tip_errs_.begin()->second, false);
}

void RegisterDialog::checkUserValid() {
    if (ui->user_edit->text() == "") {
        addTipErr(TipErr::kUserErr, tr("用户名不能为空"));
        return;
    }

    delTipErr(TipErr::kUserErr);
}

void RegisterDialog::checkEmailValid() {
    auto email = ui->email_edit->text();

    QRegularExpression regex(kEmailRegex.data());
    bool match = regex.match(email).hasMatch();
    if (!match) {
        addTipErr(TipErr::kEmailErr, tr("邮箱地址不正确"));
        return;
    }

    delTipErr(TipErr::kEmailErr);
}

void RegisterDialog::checkPasswordValid() {
    qDebug() << "checkPasswordValid";

    auto password = ui->password_edit->text();
    if (password.length() < 6 || password.length() > 15) {
        addTipErr(TipErr::kPasswordErr, tr("密码长度应为6-15"));
        return;
    }

    QRegularExpression regex(kPasswordRegex.data());
    bool match = regex.match(password).hasMatch();
    if (!match) {
        addTipErr(TipErr::kPasswordErr, tr("不能包含非法字符"));
        return;
    }

    delTipErr(TipErr::kPasswordErr);

    checkConfirmValid();
}

void RegisterDialog::checkConfirmValid() {
    auto password = ui->password_edit->text();
    auto confirm = ui->confirm_edit->text();
    if (password != confirm) {
        addTipErr(TipErr::kPasswordConfirm, tr("密码和确认密码不匹配"));
        return;
    }

    delTipErr(TipErr::kPasswordConfirm);
}

void RegisterDialog::checkVerifyValid() {
    if (ui->verify_edit->text() == "") {
        addTipErr(TipErr::kVerifyErr, tr("验证码不能为空"));
        return;
    }

    delTipErr(TipErr::kUserErr);
}
