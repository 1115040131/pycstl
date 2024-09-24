#pragma once

#include <QDialog>

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

signals:
    void switchRegister();
    void switchReset();

private slots:
    // 忘记密码
    void slot_forget_password();

    // 登录按钮点击
    void on_login_btn_clicked();

    // 注册按钮点击
    void on_reg_btn_clicked();

private:
    // 初始化头像
    void initHead();

private:
    Ui::LoginDialog* ui;
};