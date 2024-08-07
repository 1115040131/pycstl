#include "chat/client/widgets/mainwindow.h"

#include "chat/client/widgets/ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    login_dialog_ = new LoginDialog();
    setCentralWidget(login_dialog_);
    login_dialog_->show();

    register_dialog_ = new RegisterDialog();

    connect(login_dialog_, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchRegister);
}

MainWindow::~MainWindow() {
    delete ui;
    delete login_dialog_;
    delete register_dialog_;
}

void MainWindow::SlotSwitchRegister() {
    setCentralWidget(register_dialog_);
    login_dialog_->hide();
    register_dialog_->show();
}