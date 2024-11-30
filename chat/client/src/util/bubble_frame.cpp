#include "chat/client/util/bubble_frame.h"

#include <QPainter>

inline constexpr int kTriangleWidth = 8;  // 三角宽度

BubbleFrame::BubbleFrame(ChatRole role, QWidget* parent) : QFrame(parent), role_(role), margin_(3) {
    h_layout_ = new QHBoxLayout();
    if (role == ChatRole::kSelf) {
        h_layout_->setContentsMargins(margin_, margin_, margin_ + kTriangleWidth, margin_);
    } else {
        h_layout_->setContentsMargins(margin_ + kTriangleWidth, margin_, margin_, margin_);
    }

    setLayout(h_layout_);
}

void BubbleFrame::setMargin(int) {}

void BubbleFrame::setWidget(QWidget* w) {
    if (h_layout_->count() > 0) {
        return;
    } else {
        h_layout_->addWidget(w);
    }
}

void BubbleFrame::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);  // 无边框

    if (role_ == ChatRole::kSelf) {
        QColor bk_color(158, 234, 106);
        painter.setBrush(QBrush(bk_color));
        // 画气泡
        QRect bk_rect = QRect(0, 0, this->width() - kTriangleWidth, this->height());
        painter.drawRoundedRect(bk_rect, 5, 5);
        // 画三角
        QPointF points[3] = {
            QPointF(bk_rect.x() + bk_rect.width(), 12),
            QPointF(bk_rect.x() + bk_rect.width(), 12 + kTriangleWidth),
            QPointF(bk_rect.x() + bk_rect.width() + kTriangleWidth, 12 + kTriangleWidth / 2),
        };
        painter.drawPolygon(points, 3);
    } else {
        // 画气泡
        QColor bk_color(Qt::white);
        painter.setBrush(QBrush(bk_color));
        QRect bk_rect = QRect(kTriangleWidth, 0, this->width() - kTriangleWidth, this->height());
        painter.drawRoundedRect(bk_rect, 5, 5);
        // 画小三角
        QPointF points[3] = {
            QPointF(bk_rect.x(), 12),
            QPointF(bk_rect.x(), 12 + kTriangleWidth),
            QPointF(bk_rect.x() - kTriangleWidth, 12 + kTriangleWidth / 2),
        };
        painter.drawPolygon(points, 3);
    }
}
