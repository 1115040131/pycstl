#pragma once

#include <QFrame>
#include <QHBoxLayout>

#include "chat/client/define.h"

class BubbleFrame : public QFrame {
public:
    explicit BubbleFrame(ChatRole role, QWidget* parent = nullptr);

    void setMargin(int margin);

    void setWidget(QWidget* w);

private:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    ChatRole role_;
    int margin_;
    QHBoxLayout* h_layout_;
};
