#include "chat/client/widget/reset_dialog.h"

#include "chat/client/api.h"
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
}

ResetDialog::~ResetDialog() { delete ui; }

void ResetDialog::on_verify_btn_clicked() {}

void ResetDialog::on_sure_btn_clicked() {}

void ResetDialog::on_return_btn_clicked() { emit switchLogin(); }

void ResetDialog::slot_reset_mod_finish(ReqId, const QString&, ErrorCode) {}

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
