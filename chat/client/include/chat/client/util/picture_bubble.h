#pragma once

#include <QPixmap>

#include "chat/client/util/bubble_frame.h"

class PictureBubble : public BubbleFrame {
    Q_OBJECT

public:
    PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent = nullptr);
};