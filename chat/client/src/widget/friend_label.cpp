#include "chat/client/widget/friend_label.h"

#include "chat/client/widget/ui_friend_label.h"

FriendLabel::FriendLabel(QWidget* parent) : QFrame(parent), ui(new Ui::FriendLabel) {
    ui->setupUi(this);

    ui->close_label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                              "selected_pressed");
    connect(ui->close_label, &ClickedLabel::clicked, this, &FriendLabel::slot_close);
}

FriendLabel::~FriendLabel() { delete ui; }

void FriendLabel::setText(const QString& text) {
    text_ = text;
    ui->tip_label->setText(text_);
    ui->tip_label->adjustSize();

    QFontMetrics font_metrics(ui->tip_label->font());  // 获取 QLabel 控件的字体信息
    // auto text_width = font_metrics.horizontalAdvance(ui->tip_label->text());  // 获取文本宽度
    auto text_height = font_metrics.height();  // 获取文本高度

    this->setFixedWidth(ui->tip_label->width() + ui->close_label->width() + 5);
    this->setFixedHeight(text_height + 5);
}

void FriendLabel::slot_close() { emit sig_close(text_); }