#include "chat/client/widget/mainwindow.h"

#include "chat/client/widget/ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    login_dialog_ = new LoginDialog(this);
    register_dialog_ = new RegisterDialog(this);

    login_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    register_dialog_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    setCentralWidget(login_dialog_);
    login_dialog_->show();
    register_dialog_->hide();

    connect(login_dialog_, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchRegister);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::SlotSwitchRegister() {
    setCentralWidget(register_dialog_);
    login_dialog_->hide();
    register_dialog_->show();
}