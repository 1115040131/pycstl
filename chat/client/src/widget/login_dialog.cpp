#include "chat/client/widget/login_dialog.h"

#include "chat/client/widget/ui_login_dialog.h"

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    // 忘记密码
    ui->forget_label->setState("normal", "hover", "", "selected", "selected_hover", "");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_password);
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::slot_forget_password() { emit switchReset(); }
