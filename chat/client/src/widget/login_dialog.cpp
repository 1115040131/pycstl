#include "chat/client/widget/login_dialog.h"

#include <QPainter>
#include <QPainterPath>

#include "chat/client/api.h"
#include "chat/client/widget/ui_login_dialog.h"

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);

    // 忘记密码
    ui->forget_label->setState("normal", "hover", "", "selected", "selected_hover", "");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_password);

    initHead();

    ui->err_tip->clear();
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::slot_forget_password() { emit switchReset(); }

void LoginDialog::on_login_btn_clicked() {
    if (ui->err_tip->checkEmailValid(ui->email_edit->text()) &&
        ui->err_tip->checkPasswordValid(ui->password_edit->text())) {
    }
}

void LoginDialog::on_reg_btn_clicked() { emit switchRegister(); }

void LoginDialog::initHead() {
    QPixmap original_pixmap("chat/client/res/head_2.jpg");
    // 设置图片自动缩放
    original_pixmap =
        original_pixmap.scaled(ui->head_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个圆角图片
    QPixmap rounded_pixmap(original_pixmap.size());
    rounded_pixmap.fill(Qt::transparent);

    QPainter painter(&rounded_pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);           // 设置抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);  // 设置平滑变换

    // 设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, original_pixmap.width(), original_pixmap.height(), 10, 10);
    painter.setClipPath(path);

    // 将原始图片绘制到圆角图片上
    painter.drawPixmap(0, 0, original_pixmap);

    ui->head_label->setPixmap(rounded_pixmap);
}
