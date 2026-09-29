#pragma once

#include <QPixmap>

import chat.client.util.bubble_frame;

class PictureBubble : public BubbleFrame {
    Q_OBJECT

public:
    PictureBubble(ChatRole role, const QPixmap& picture, QWidget* parent = nullptr);
};