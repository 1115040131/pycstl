#include "chat/client/widget/mainwindow.h"

#include "chat/client/widget/login_dialog.h"
#include "chat/client/widget/register_dialog.h"
#include "chat/client/widget/reset_dialog.h"
#include "chat/client/widget/ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    SlotSwitchLogin();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::SlotSwitchRegister() {
    register_dialog_ = new RegisterDialog(this);
    register_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    setCentralWidget(register_dialog_);

    // 注册界面返回登录
    connect(register_dialog_, &RegisterDialog::switchLogin, this, &MainWindow::SlotSwitchLogin);
}

void MainWindow::SlotSwitchLogin() {
    login_dialog_ = new LoginDialog(this);
    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    setCentralWidget(login_dialog_);

    // 登录界面点击注册
    connect(login_dialog_, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchRegister);

    // 连接登陆界面忘记密码信号
    connect(login_dialog_, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset() {
    reset_dialog_ = new ResetDialog(this);
    reset_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    setCentralWidget(reset_dialog_);

    // 重置界面返回登录
    connect(reset_dialog_, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin);
}