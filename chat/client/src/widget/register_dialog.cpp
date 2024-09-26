#include "chat/client/widget/register_dialog.h"

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/widget/ui_register_dialog.h"

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    // 连接输入框的错误信号
    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkUserValid(ui->user_edit->text()); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkEmailValid(ui->email_edit->text()); });
    connect(ui->password_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkPasswordValid(ui->password_edit->text()); });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkConfirmValid(ui->password_edit->text(), ui->confirm_edit->text()); });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this,
            [this]() { ui->err_tip->checkVerifyValid(ui->verify_edit->text()); });

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
    if (ui->err_tip->checkEmailValid(email)) {
        // 发送验证码
        QJsonObject root;
        root["email"] = email;
        HttpMgr::GetInstance().PostHttpRequest(kModule, ReqId::kGetVerifyCode, root);
    }
}

void RegisterDialog::on_sure_btn_clicked() {
    if (ui->err_tip->checkUserValid(ui->user_edit->text()) &&
        ui->err_tip->checkEmailValid(ui->email_edit->text()) &&
        ui->err_tip->checkPasswordValid(ui->password_edit->text()) &&
        ui->err_tip->checkConfirmValid(ui->password_edit->text(), ui->confirm_edit->text()) &&
        ui->err_tip->checkVerifyValid(ui->verify_edit->text())) {
        QJsonObject root;
        root["user"] = ui->user_edit->text();
        root["email"] = ui->email_edit->text();
        root["password"] = xorString(ui->password_edit->text());
        root["confirm"] = xorString(ui->confirm_edit->text());
        root["verify_code"] = ui->verify_edit->text();
        HttpMgr::GetInstance().PostHttpRequest(kModule, ReqId::kRegUser, root);
    }
}

void RegisterDialog::on_cancel_btn_clicked() { emit switchLogin(); }

void RegisterDialog::on_return_btn_clicked() {
    countdown_timer_->stop();
    emit switchLogin();
}

void RegisterDialog::slot_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err) {
    auto json_doc = ui->err_tip->checkHttpResponse(res, err);
    if (json_doc) {
        handlers_[req_id](json_doc->object());
    }
}

void RegisterDialog::initHttpHandlers() {
    handlers_.emplace(ReqId::kGetVerifyCode, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            ui->err_tip->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        ui->err_tip->showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "kGetVerifyCode finish: email: " << email;
    });
    handlers_.emplace(ReqId::kRegUser, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            ui->err_tip->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        ui->err_tip->showTip(tr("用户注册成功"), true);
        qDebug() << "kRegUser finish: email: " << email << " user uid: " << json["uid"].toInt();

        changeTipPage();
    });
}

void RegisterDialog::changeTipPage() {
    countdown_timer_->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    countdown_timer_->start(1000);
}
