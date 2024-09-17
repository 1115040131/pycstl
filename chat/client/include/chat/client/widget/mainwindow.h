#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class LoginDialog;
class RegisterDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void SlotSwitchRegister();

    void SlotSwitchLogin();

private:
    Ui::MainWindow* ui;
    LoginDialog* login_dialog_;
    RegisterDialog* register_dialog_;
};
