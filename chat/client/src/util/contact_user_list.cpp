#include "chat/client/util/contact_user_list.h"

#include <QEvent>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QWheelEvent>

#include "chat/client/tcp_mgr.h"
#include "chat/client/widget/contact_user_item.h"
#include "chat/client/widget/group_tip_item.h"

ContactUserList::ContactUserList(QWidget* parent) : QListWidget(parent) {
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);

    // 模拟从后端传来的数据, 进行列表加载
    // TODO: 用后端的数据进行加载
    addContactUserList();

    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);

    // 由 chat_dialog 转发的信号
    // 连接自己同意后界面刷新
    // connect(&TcpMgr::GetInstance(), TcpMgr::sig_auth_rsp, this, &ContactUserList::slot_auth_rsp);

    // 连接对端同意后通知的信号
    // connect(&TcpMgr::GetInstance(), TcpMgr::sig_add_auth_friend, this, &ContactUserList::slot_add_auth_friend);
}

bool ContactUserList::eventFilter(QObject* watched, QEvent* event) {
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
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
                emit sig_loading_contact_user();
            }

            return true;
        }
    }
    return QListWidget::eventFilter(watched, event);
}

void ContactUserList::showRedPoint(bool show) { add_friend_item_->showRedPoint(show); }

void ContactUserList::addContactUserList() {
    {
        auto group_tip_item = new GroupTipItem;
        auto item = new QListWidgetItem;
        item->setSizeHint(group_tip_item->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, group_tip_item);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }
    {
        add_friend_item_ = new ContactUserItem;
        add_friend_item_->setObjectName("add_friend_item");
        add_friend_item_->setInfo(0, "新的朋友", "chat/client/res/add_friend.png");
        add_friend_item_->SetItemType(ListItemBase::Type::kApplyFriendItem);
        auto item = new QListWidgetItem;
        item->setSizeHint(add_friend_item_->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, add_friend_item_);
        // 默认设置新的朋友申请条目被选中
        this->setCurrentItem(item);
    }
    {
        auto group_tip_item = new GroupTipItem;
        group_tip_item->setGroupTip("联系人");
        group_item_ = new QListWidgetItem;
        group_item_->setSizeHint(group_tip_item->sizeHint());
        this->addItem(group_item_);
        this->setItemWidget(group_item_, group_tip_item);
        group_item_->setFlags(group_item_->flags() & ~Qt::ItemIsSelectable);
    }
    for (int i = 0; i < 13; i++) {
        int random_value = QRandomGenerator::global()->bounded(100);
        int name_index = random_value % names.size();
        int head_index = random_value % heads.size();
        // int msg_index = random_value % strs.size();

        auto contact_user_item = new ContactUserItem;
        contact_user_item->setInfo(0, names[name_index], heads[head_index]);
        auto item = new QListWidgetItem;
        item->setSizeHint(contact_user_item->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, contact_user_item);
    }
}

void ContactUserList::addAuthFriendItem(const std::shared_ptr<AuthInfo>& auth_info) {
    // 在 groupitem 之后插入新项
    int random_value = QRandomGenerator::global()->bounded(100);
    int head_index = random_value % heads.size();

    auto contact_user_item = new ContactUserItem;
    contact_user_item->setInfo(auth_info->uid, auth_info->name, heads[head_index]);
    auto item = new QListWidgetItem;
    item->setSizeHint(contact_user_item->sizeHint());

    // 获取 groupitem 的索引
    int index = this->row(group_item_);
    this->insertItem(index + 1, item);
    this->setItemWidget(item, contact_user_item);
}

void ContactUserList::slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_info) { addAuthFriendItem(auth_info); }

void ContactUserList::slot_add_auth_friend(const std::shared_ptr<AuthInfo>& auth_info) {
    addAuthFriendItem(auth_info);
}

void ContactUserList::slot_item_clicked(QListWidgetItem* item) {
    auto widget = this->itemWidget(item);
    if (!widget) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto custom_item = qobject_cast<ListItemBase*>(widget);
    if (!custom_item) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto item_type = custom_item->GetItemType();
    if (item_type == ListItemBase::Type::kInvalidItem || item_type == ListItemBase::Type::kGroupTipItem) {
        qDebug() << "slot invalid item clicked";
    } else if (item_type == ListItemBase::Type::kApplyFriendItem) {
        qDebug() << "apply friend item clicked";
        showRedPoint(false);  // 消除红点
        emit sig_switch_apply_friend_page();
    } else if (item_type == ListItemBase::Type::kContactUserItem) {
        qDebug() << "contact user item clicked";
        emit sig_switch_friend_info_page();
    }
}
