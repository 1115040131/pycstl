#include "chat/client/util/chat_user_list.h"

#include <QEvent>
#include <QScrollBar>
#include <QWheelEvent>

ChatUserList::ChatUserList(QWidget* parent) : QListWidget(parent) {
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject* watched, QEvent* event) {
    if (this->viewport() == watched) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮, 显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开, 隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else if (event->type() == QEvent::Wheel) {
            // 滚动事件, 显示滚动条
            auto wheel_event = static_cast<QWheelEvent*>(event);

            int num_degrees = wheel_event->angleDelta().y() / 8;
            int num_steps = num_degrees / 15;  // 计算滚动步数

            // 设置滚动幅度
            QScrollBar* scroll_bar = this->verticalScrollBar();
            scroll_bar->setValue(scroll_bar->value() - num_steps);

            // 检查是否滚到底部
            if (scroll_bar->value() >= scroll_bar->maximum()) {
                emit sig_loading_chat_user();
            }

            return true;
        }
    }

    return QListWidget::eventFilter(watched, event);
}
