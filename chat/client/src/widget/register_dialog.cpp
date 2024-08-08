#include "chat/client/widget/register_dialog.h"

#include <QJsonDocument>
#include <QRegularExpression>

#include "chat/client/api.h"
#include "chat/client/http_mgr.h"
#include "chat/client/widget/ui_register_dialog.h"

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->repeat_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    Repolish(ui->err_tip);

    connect(&HttpMgr::GetInstance(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    initHttpHandlers();
}

RegisterDialog::~RegisterDialog() { delete ui; }

void RegisterDialog::on_get_code_btn_clicked() {
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if (match) {
        // 发送验证码
    } else {
        showTip(tr("邮箱地址不正确"), false);
    }
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
    handlers_.insert(ReqId::kGetVarifyCode, [this](const QJsonObject& json) {
        auto error = static_cast<ErrorCode>(json["error"].toInt());
        if (error != ErrorCode::kSuccess) {
            this->showTip(tr("参数错误"), false);
            return;
        }

        auto email = json["email"].toString();
        this->showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "email is " << email;
    });
    // handlers_.insert(ReqId::kRegUser, [this](const QJsonObject& json) {});
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
