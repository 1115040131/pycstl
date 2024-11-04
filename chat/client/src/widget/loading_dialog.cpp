#include "chat/client/widget/loading_dialog.h"

#include <QMovie>

#include "chat/client/widget/ui_loading_dialog.h"

LoadingDialog::LoadingDialog(QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint),
      ui(new Ui::LoadingDialog) {
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);  // 设置背景透明
    setFixedSize(parent->size());                // 设置对话框为全屏尺寸
    setWindowModality(Qt::WindowModal);          // 让对话框总是显示在父窗口之上

    QMovie* movie = new QMovie("chat/client/res/loading.gif");  // 加载动画的资源文件
    ui->loading_label->setMovie(movie);
    movie->start();
}

LoadingDialog::~LoadingDialog() { delete ui; }
