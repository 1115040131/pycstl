#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class LoginDialog;
class RegisterDialog;
class ResetDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // 切换到注册界面
    void SlotSwitchRegister();

    // 切换到登录界面
    void SlotSwitchLogin();

    // 切换到忘记密码界面
    void SlotSwitchReset();

private:
    Ui::MainWindow* ui;
    LoginDialog* login_dialog_;
    RegisterDialog* register_dialog_;
    ResetDialog* reset_dialog_;
};
