#include "chat/client/util/chat_view.h"

#include <QEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

ChatView::ChatView(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setContentsMargins(0, 0, 0, 0);

    scroll_area_ = new QScrollArea();
    scroll_area_->setObjectName("chat_area");
    main_layout->addWidget(scroll_area_);

    QWidget* w = new QWidget(this);
    w->setObjectName("chat_background");
    w->setAutoFillBackground(true);

    QVBoxLayout* v_layout_1 = new QVBoxLayout();
    v_layout_1->addWidget(new QWidget(), 100000);
    w->setLayout(v_layout_1);
    scroll_area_->setWidget(w);

    scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto scroll_bar = scroll_area_->verticalScrollBar();
    connect(scroll_bar, &QScrollBar::rangeChanged, this, &ChatView::onVerticalScrollBarMoved);

    // 把垂直 ScrollBar 放到上边
    QVBoxLayout* v_layout_2 = new QVBoxLayout();
    v_layout_2->addWidget(scroll_bar, 0, Qt::AlignRight);
    v_layout_2->setContentsMargins(0, 0, 0, 0);
    scroll_area_->setLayout(v_layout_2);
    scroll_bar->setHidden(true);

    scroll_area_->setWidgetResizable(true);
    scroll_area_->installEventFilter(this);
    initStyleSheet();
}

void ChatView::appendChatItem(QWidget* item) {
    auto v_layout = qobject_cast<QVBoxLayout*>(scroll_area_->widget()->layout());
    v_layout->insertWidget(v_layout->count() - 1, item);
    is_appended = true;
}

void ChatView::prependChatItem(QWidget* item) { (void)item; }

void ChatView::insertChatItem(QWidget* position, QWidget* item) {
    (void)position;
    (void)item;
}

void ChatView::initStyleSheet() {}

bool ChatView::eventFilter(QObject* watched, QEvent* event) {
    if (watched == scroll_area_) {
        if (event->type() == QEvent::Enter) {
            scroll_area_->verticalScrollBar()->setHidden(scroll_area_->verticalScrollBar()->maximum() == 0);
        } else if (event->type() == QEvent::Leave) {
            scroll_area_->verticalScrollBar()->setHidden(true);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChatView::onVerticalScrollBarMoved(int, int) {
    if (is_appended) {  // 添加 item 可能调用多次
        auto scroll_bar = scroll_area_->verticalScrollBar();
        scroll_bar->setSliderPosition(scroll_bar->maximum());
        QTimer::singleShot(500, [this]() { is_appended = false; });  // 500 ms 最多调用一次
    }
}
