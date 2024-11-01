#include "chat/client/util/picture_bubble.h"

#include <QLabel>

inline constexpr int kPictureMaxWidth = 160;
inline constexpr int kPictureMaxHeight = 90;

PictureBubble::PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent) : BubbleFrame(role, parent) {
    QLabel* lb = new QLabel();
    lb->setScaledContents(true);
    QPixmap pix = picture.scaled(QSize(kPictureMaxWidth, kPictureMaxHeight), Qt::KeepAspectRatio);
    lb->setPixmap(pix);
    this->setWidget(lb);

    int left_margin = this->layout()->contentsMargins().left();
    int right_margin = this->layout()->contentsMargins().right();
    int v_margin = this->layout()->contentsMargins().bottom();
    setFixedSize(pix.width() + left_margin + right_margin, pix.height() + v_margin * 2);
}
