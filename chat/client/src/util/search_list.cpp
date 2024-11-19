#include "chat/client/util/search_list.h"

#include <QEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QWheelEvent>

#include "chat/client/tcp_mgr.h"
#include "chat/client/util/customize_edit.h"
#include "chat/client/widget/add_user_item.h"
#include "chat/client/widget/find_fail_dialog.h"
#include "chat/client/widget/find_success_dialog.h"
#include "chat/client/widget/loading_dialog.h"

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

void SearchList::setSearchEdit(CustomizeEdit* edit) { search_edit_ = edit; }

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

void SearchList::waitPending(bool pending) {
    if (pending) {
        loading_dialog_ = new LoadingDialog(this);            // 将当前对话框设置为父对象
        loading_dialog_->setAttribute(Qt::WA_DeleteOnClose);  // 对话框关闭时自动删除
        loading_dialog_->show();                              // 显示悬浮对话框
        send_pending_ = true;
    } else {
        loading_dialog_->deleteLater();  // 加载完成后关闭对话框
        send_pending_ = false;
    }
}

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

void SearchList::closeFindDialog() {
    if (find_dialog_) {
        find_dialog_.reset();
    }
}

void SearchList::slot_item_clicked(QListWidgetItem* item) {
    auto widget = this->itemWidget(item);  // 获取自定义 widget 对象
    if (!widget) {
        qDebug() << "widget is nullptr";
        return;
    }

    auto custom_item = qobject_cast<ListItemBase*>(widget);
    if (!custom_item) {
        qDebug() << "widget is not ListItemBase*";
        return;
    }

    auto item_type = custom_item->GetItemType();
    if (item_type == ListItemBase::Type::kInvalidItem) {
        qDebug() << "slot invalid item clicked";
        return;
    }

    if (item_type == ListItemBase::Type::kAddUserTipItem) {
        if (send_pending_) {
            return;
        }

        if (!search_edit_) {
            qDebug() << "search_edit_ nullptr";
            return;
        }

        waitPending(true);
        auto uid_str = search_edit_->text();

        QJsonObject root;
        root["uid"] = uid_str;

        QJsonDocument doc(root);
        emit TcpMgr::GetInstance().sig_send_data(ReqId::kSearchUserReq, doc.toJson(QJsonDocument::Compact));

        return;
    }

    // 清除弹出框
    closeFindDialog();
}

void SearchList::slot_user_search(const std::shared_ptr<SearchInfo>& search_info) {
    waitPending(false);
    if (!search_info) {
        find_dialog_ = std::make_unique<FindFailDialog>(this);
    } else {
        find_dialog_ = std::make_unique<FindSuccessDialog>(this);
        static_cast<FindSuccessDialog*>(find_dialog_.get())->setSearchInfo(search_info);

        // TODO: 查找到已经是好友
    }
    find_dialog_->show();
}
