#include "chat/client/widget/find_fail_dialog.h"

#include "chat/client/widget/ui_find_fail_dialog.h"

FindFailDialog::FindFailDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FindFailDialog) {
    ui->setupUi(this);

    setWindowTitle("添加");
    setWindowFlag(Qt::FramelessWindowHint);  // 隐藏对话框标题
    this->setObjectName("FindFailDialog");

    setWindowModality(Qt::WindowModal);
}

FindFailDialog::~FindFailDialog() {
    qDebug() << "~FindFailDialog";
    delete ui;
}

void FindFailDialog::on_fail_sure_btn_clicked() { this->hide(); }
