#include "chat/client/widget/register_dialog.h"

#include <QJsonDocument>

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/url_mgr.h"
#include "chat/client/widget/ui_register_dialog.h"

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    // 连接 http 请求完成信号
    initHttpHandlers();
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    // 连接输入框的错误信号
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkUserValid(ui->user_edit->text())); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkEmailValid(ui->email_edit->text())); });
    connect(ui->password_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkPasswordValid(ui->password_edit->text())); });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this]() {
        showErrTip(input_check_.checkConfirmValid(ui->password_edit->text(), ui->confirm_edit->text()));
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this,
            [this]() { showErrTip(input_check_.checkVerifyValid(ui->verify_edit->text())); });

    // 密码显示
    ui->password_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover", "");
    ui->confirm_visible->setState("unvisible", "unvisible_hover", "", "visible", "visible_hover", "");
    connect(ui->password_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->password_visible->getState();
        if (state == ClickedLabel::State::kNormal) {
            ui->password_edit->setEchoMode(QLineEdit::Password);
        } else {
            ui->password_edit->setEchoMode(QLineEdit::Normal);
        }
    });
    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->getState();
        if (state == ClickedLabel::State::kNormal) {
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        } else {
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    // 注册成功, 倒计时返回登录
    countdown_timer_ = std::make_unique<QTimer>(this);
    connect(countdown_timer_.get(), &QTimer::timeout, [this]() {
        if (countdown_ == 0) {
            countdown_timer_->stop();
            emit switchLogin();
            return;
        }
        countdown_--;
        auto str = QString("注册成功，%1s后返回登录").arg(countdown_);
        ui->tip_label->setText(str);
    });

    // 连接 http 请求完成信号
    initHttpHandlers();
    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
}

RegisterDialog::~RegisterDialog() { delete ui; }

void RegisterDialog::on_get_code_btn_clicked() {
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
                                           ReqId::kGetVerifyCode, Module::kRegisterMod);
}

void RegisterDialog::on_sure_btn_clicked() {
    if (input_check_.checkUserValid(ui->user_edit->text()) ||
        input_check_.checkEmailValid(ui->email_edit->text()) ||
        input_check_.checkPasswordValid(ui->password_edit->text()) ||
        input_check_.checkConfirmValid(ui->password_edit->text(), ui->confirm_edit->text()) ||
        input_check_.checkVerifyValid(ui->verify_edit->text())) {
        showErrTip(input_check_.getLastError());
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

void RegisterDialog::on_cancel_btn_clicked() { emit switchLogin(); }

void RegisterDialog::on_return_btn_clicked() {
    countdown_timer_->stop();
    emit switchLogin();
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

        changeTipPage();
    });
}

void RegisterDialog::showTip(const QString& str, bool normal) {
    ui->err_tip->setText(str);
    if (normal) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    repolish(ui->err_tip);
}

void RegisterDialog::showErrTip(std::optional<std::string_view> str) {
    if (str) {
        showTip(tr(str->data()), false);
    } else {
        ui->err_tip->clear();
    }
}

void RegisterDialog::changeTipPage() {
    countdown_timer_->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    countdown_timer_->start(1000);
}
