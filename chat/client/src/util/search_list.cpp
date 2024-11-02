#include "chat/client/util/search_list.h"

#include <QEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include "chat/client/tcp_mgr.h"
#include "chat/client/widget/add_user_item.h"

SearchList::SearchList(QWidget* parent) : QListWidget(parent) {
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 安装事件过滤器
    this->viewport()->installEventFilter(this);

    // 链接点击
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);

    // 添加条目
    addTipItem();

    // 链接搜索条目
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}

void SearchList::closeFindDialog() {}

void SearchList::setSearchEdit(QWidget* edit) { (void)edit; }

bool SearchList::eventFilter(QObject* watched, QEvent* event) {
    if (watched == this->viewport()) {
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

            return true;
        }
    }

    return QListWidget::eventFilter(watched, event);
}

void SearchList::waitPending(bool pending) { (void)pending; }

void SearchList::addTipItem() {
    {
        auto invalid_item = new QWidget;
        invalid_item->setObjectName("invalid_item");
        auto item = new QListWidgetItem;
        item->setSizeHint(QSize(250, 10));
        this->addItem(item);
        this->setItemWidget(item, invalid_item);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }
    {
        auto add_user_item = new AddUserItem;
        auto item = new QListWidgetItem;
        item->setSizeHint(add_user_item->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, add_user_item);
    }
}

void SearchList::slot_item_clicked(QListWidgetItem* item) { (void)item; }

void SearchList::slot_user_search(const SearchInfo& search_info) { (void)search_info; }
