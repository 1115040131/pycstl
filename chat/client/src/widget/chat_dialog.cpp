#include "chat/client/widget/chat_dialog.h"

#include <QEvent>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTimer>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
#include "chat/client/widget/chat_user_widget.h"
#include "chat/client/widget/contact_user_item.h"
#include "chat/client/widget/loading_dialog.h"
#include "chat/client/widget/ui_chat_dialog.h"

ChatDialog::ChatDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ChatDialog) {
    ui->setupUi(this);

    // 创建一个搜索图标
    search_action_ = new QAction(ui->search_edit);
    search_action_->setIcon(QIcon("chat/client/res/search.png"));
    ui->search_edit->addAction(search_action_, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(tr("搜索"));

    // 创建一个清除图标
    clear_action_ = new QAction(ui->search_edit);
    clear_action_->setIcon(QIcon("chat/client/res/close_transparent.png"));
    ui->search_edit->addAction(clear_action_, QLineEdit::TrailingPosition);
    // 链接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_search_text_changed);
    // 点击清除图标清空输入框
    connect(clear_action_, &QAction::triggered, [this]() {
        ui->search_edit->clear();
        ShowSearch(false);
    });

    ui->search_edit->setMaxLength(15);
    ShowSearch(false);

    // 为searchlist 设置search edit
    ui->search_list->setSearchEdit(ui->search_edit);

    // 添加聊天用户和好友列表
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this, &ChatDialog::slot_loading_chat_user);
    connect(ui->contact_user_list, &ContactUserList::sig_loading_contact_user, this,
            &ChatDialog::slot_loading_contact_user);
    addChatUserList();
    addContactUserList();

    // 侧边栏
    QPixmap pixmap(UserMgr::GetInstance().GetIcon());
    ui->side_head_label->setPixmap(pixmap);
    ui->side_head_label->setScaledContents(true);  // 自动缩放图片内容以适应
    // QPixmap scaled_pixmap = pixmap.scaled(ui->side_head_label->size(), Qt::KeepAspectRatio);
    // ui->side_head_label->setPixmap(scaled_pixmap);

    ui->side_chat_label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                                  "selected_pressed");
    ui->side_chat_label->setSelected(ClickedLabel::State::kSelected);
    ui->side_contact_label->setState("normal", "hover", "pressed", "selected_normal", "selected_hover",
                                     "selected_pressed");

    addLabelGroup(ui->side_chat_label);
    addLabelGroup(ui->side_contact_label);

    connect(ui->side_chat_label, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_label, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    // 检测点击鼠标位置是否需要清空搜索框
    this->installEventFilter(this);

    // 收到服务器消息
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_friend_apply);
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_auth_rsp, this, &ChatDialog::slot_auth_rsp);
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);
    connect(&TcpMgr::GetInstance(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);

    // 连接 search_list 跳转聊天界面信号
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    // 连接 contact_user_list 跳转界面
    connect(ui->contact_user_list, &ContactUserList::sig_switch_apply_friend_page, this,
            &ChatDialog::slot_switch_apply_friend_page);
    connect(ui->contact_user_list, &ContactUserList::sig_switch_friend_info_page, this,
            &ChatDialog::slot_switch_friend_info_page);

    // 连接 friend_info_page 跳转界面
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    // 设置中心部件
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    setSelectChatItem(0);
    setSelectChatPage(0);

    // 连接 chat_user_list 点击信号
    connect(ui->chat_user_list, &ChatUserList::itemClicked, this, &ChatDialog::slot_chat_item_clicked);

    // 连接 chat_page 发送消息信号
    connect(ui->chat_page, &ChatPage::sig_append_chat_msg, this, &ChatDialog::slot_append_send_chat_msg);
}

ChatDialog::~ChatDialog() { delete ui; }

bool ChatDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouse_event = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouse_event);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::ShowSearch(bool is_search) {
    if (is_search) {
        ui->chat_user_list->hide();
        ui->contact_user_list->hide();
        ui->search_list->show();
        mode_ = UIMode::kSearchMode;
    } else if (state_ == UIMode::kChatMode) {
        ui->chat_user_list->show();
        ui->contact_user_list->hide();
        ui->search_list->hide();
        mode_ = UIMode::kChatMode;
    } else if (state_ == UIMode::kContactMode) {
        ui->chat_user_list->hide();
        ui->contact_user_list->show();
        ui->search_list->hide();
        mode_ = UIMode::kContactMode;
    }
}

void ChatDialog::addChatUserList() {
    auto friend_list = UserMgr::GetInstance().LoadChatListPerPage();
    qDebug() << friend_list.size();
    for (const auto& friend_info : friend_list) {
        if (chat_item_added_.count(friend_info->uid) > 0) {
            continue;
        }

        auto chat_user_widget = new ChatUserWidget;
        auto user_info = std::make_shared<UserInfo>(UserInfo::FromFriendInfo(*friend_info));
        chat_user_widget->SetInfo(user_info);
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(chat_user_widget->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_widget);

        chat_item_added_[friend_info->uid] = item;
    }

    static bool test = true;
    if (test) {
        test = false;
        for (int i = 0; i < 13; i++) {
            int random_value = QRandomGenerator::global()->bounded(100);
            int name_index = random_value % names.size();
            int head_index = random_value % heads.size();
            int msg_index = random_value % strs.size();

            auto chat_user_widget = new ChatUserWidget;
            auto user_info =
                std::make_shared<UserInfo>(200 + i, 0, names[name_index], "", heads[head_index], strs[msg_index]);
            chat_user_widget->SetInfo(user_info);
            QListWidgetItem* item = new QListWidgetItem();
            item->setSizeHint(chat_user_widget->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_widget);
        }
    }
}

void ChatDialog::addContactUserList() {
    auto friend_list = UserMgr::GetInstance().LoadContactListPerPage();
    for (const auto& friend_info : friend_list) {
        auto contact_user_item = new ContactUserItem;
        contact_user_item->setInfo(friend_info);
        auto item = new QListWidgetItem;
        item->setSizeHint(contact_user_item->sizeHint());
        ui->contact_user_list->addItem(item);
        ui->contact_user_list->setItemWidget(item, contact_user_item);
    }
    static bool test = true;
    if (test) {
        test = false;
        for (int i = 0; i < 13; i++) {
            int random_value = QRandomGenerator::global()->bounded(100);
            int name_index = random_value % names.size();
            int head_index = random_value % heads.size();
            // int msg_index = random_value % strs.size();

            auto contact_user_item = new ContactUserItem;
            contact_user_item->setInfo(0, names[name_index], heads[head_index]);
            auto item = new QListWidgetItem;
            item->setSizeHint(contact_user_item->sizeHint());
            ui->contact_user_list->addItem(item);
            ui->contact_user_list->setItemWidget(item, contact_user_item);
        }
    }
}

void ChatDialog::addLabelGroup(StateWidget* label) { labels_.push_back(label); }

void ChatDialog::clearLabelState(StateWidget* select_label) {
    for (const auto& label : labels_) {
        if (label != select_label) {
            label->setSelected(ClickedLabel::State::kNormal);
        } else {
            label->setSelected(ClickedLabel::State::kSelected);
        }
    }
}

void ChatDialog::handleGlobalMousePress(QMouseEvent* event) {
    // 判断是否处于搜索模式
    if (mode_ != UIMode::kSearchMode) {
        return;
    }

    // 判断鼠标点击位置是否在搜索列表外
    QPoint position = ui->search_list->mapFromGlobal(event->globalPosition().toPoint());
    if (!ui->search_list->rect().contains(position)) {
        ui->search_edit->clear();
        ShowSearch(false);
    }
}

void ChatDialog::addAuthFriend(const std::shared_ptr<AuthInfo>& auth_info) {
    auto chat_user_widget = new ChatUserWidget;
    auto user_info = std::make_shared<UserInfo>(UserInfo::FromAuthInfo(*auth_info));
    chat_user_widget->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(chat_user_widget->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_widget);

    chat_item_added_[user_info->uid] = item;
}

void ChatDialog::setSelectChatItem(int uid) {
    if (ui->chat_user_list->count() == 0) {
        return;
    }

    if (uid == 0) {
        ui->chat_user_list->setCurrentRow(0);
        auto first_item = ui->chat_user_list->item(0);
        if (!first_item) {
            qDebug() << __func__ << "first_item is nullptr";
            return;
        }

        // 转为 Widget
        auto widget = ui->chat_user_list->itemWidget(first_item);
        if (!widget) {
            qDebug() << __func__ << "widget is nullptr";
            return;
        }

        auto chat_user_widget = qobject_cast<ChatUserWidget*>(widget);
        if (!chat_user_widget) {
            qDebug() << __func__ << "chat_user_widget is nullptr";
            return;
        }

        current_chat_uid_ = chat_user_widget->GetUserInfo()->uid;
        return;
    }

    auto iter = chat_item_added_.find(uid);
    if (iter == chat_item_added_.end()) {
        ui->chat_user_list->setCurrentRow(0);
        return;
    }

    ui->chat_user_list->setCurrentItem(iter->second);
    current_chat_uid_ = uid;
}

void ChatDialog::setSelectChatPage(int uid) {
    if (ui->chat_user_list->count() == 0) {
        return;
    }

    QListWidgetItem* item = nullptr;
    if (uid == 0) {
        item = ui->chat_user_list->item(0);
    } else {
        auto iter = chat_item_added_.find(uid);
        if (iter != chat_item_added_.end()) {
            item = iter->second;
        }
    }
    if (!item) {
        qDebug() << __func__ << "item is nullptr";
        return;
    }

    auto widget = ui->chat_user_list->itemWidget(item);
    if (!widget) {
        qDebug() << __func__ << "widget is nullptr";
        return;
    }

    auto chat_user_widget = qobject_cast<ChatUserWidget*>(widget);
    if (!chat_user_widget) {
        qDebug() << __func__ << "chat_user_widget is nullptr";
        return;
    }

    auto user_info = chat_user_widget->GetUserInfo();
    ui->chat_page->setUserInfo(user_info);
    return;
}

void ChatDialog::updateChatMsg(const std::vector<std::shared_ptr<TextChatData>>& chat_msgs) {
    if (chat_msgs.empty()) {
        qDebug() << __func__ << "chat_msgs empty";
    }

    for (const auto& chat_msg : chat_msgs) {
        if (chat_msg->from_uid != current_chat_uid_) {
            break;
        }
        ui->chat_page->appendChatMsg(chat_msg);
    }
}

void ChatDialog::slot_search_text_changed(const QString& text) {
    if (text.isEmpty()) {
        clear_action_->setIcon(QIcon("chat/client/res/close_transparent.png"));
    } else {
        clear_action_->setIcon(QIcon("chat/client/res/close_search.png"));
        ShowSearch(true);
    }
}

void ChatDialog::slot_loading_chat_user() {
    if (is_loading_) {
        return;
    }

    is_loading_ = true;

    LoadingDialog* loading_dialog = new LoadingDialog(this);          // 将当前对话框设置为父对象
    loading_dialog->setAttribute(Qt::WA_DeleteOnClose);               // 对话框关闭时自动删除
    loading_dialog->show();                                           // 显示悬浮对话框
    QTimer::singleShot(500, loading_dialog, &LoadingDialog::accept);  // for debug
    addChatUserList();
    // loading_dialog->deleteLater();  // 加载完成后关闭对话框

    is_loading_ = false;
}

void ChatDialog::slot_loading_contact_user() {
    if (is_loading_) {
        return;
    }

    is_loading_ = true;

    LoadingDialog* loading_dialog = new LoadingDialog(this);          // 将当前对话框设置为父对象
    loading_dialog->setAttribute(Qt::WA_DeleteOnClose);               // 对话框关闭时自动删除
    loading_dialog->show();                                           // 显示悬浮对话框
    QTimer::singleShot(500, loading_dialog, &LoadingDialog::accept);  // for debug
    addContactUserList();
    // loading_dialog->deleteLater();  // 加载完成后关闭对话框

    is_loading_ = false;
}

void ChatDialog::slot_side_chat() {
    qDebug() << "receive side chat clicked";
    clearLabelState(ui->side_chat_label);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    state_ = UIMode::kChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact() {
    qDebug() << "receive side contact clicked";
    clearLabelState(ui->side_contact_label);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    state_ = UIMode::kContactMode;
    ShowSearch(false);
    ui->side_contact_label->ShowRedPoint(false);  // 消除红点
}

void ChatDialog::slot_friend_apply(std::shared_ptr<ApplyInfo> apply) {
    if (UserMgr::GetInstance().AlreadyApply(apply->uid)) {
        return;
    }
    UserMgr::GetInstance().AddApplyList(apply);
    ui->side_contact_label->ShowRedPoint(true);
    ui->contact_user_list->showRedPoint(true);
    ui->friend_apply_page->addNewApply(apply);
}

void ChatDialog::slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_info) {
    if (UserMgr::GetInstance().CheckFriendById(auth_info->uid)) {
        return;
    }
    UserMgr::GetInstance().AddFriend(auth_info);

    addAuthFriend(auth_info);

    ui->contact_user_list->slot_auth_rsp(auth_info);
}

void ChatDialog::slot_add_auth_friend(const std::shared_ptr<AuthInfo>& auth_info) {
    if (UserMgr::GetInstance().CheckFriendById(auth_info->uid)) {
        return;
    }
    UserMgr::GetInstance().AddFriend(auth_info);

    addAuthFriend(auth_info);

    ui->contact_user_list->slot_add_auth_friend(auth_info);
}

void ChatDialog::slot_text_chat_msg(const std::vector<std::shared_ptr<TextChatData>>& chat_msgs) {
    if (chat_msgs.empty()) {
        qDebug() << __func__ << "chat_msgs empty";
    }

    auto from_uid = chat_msgs.front()->from_uid;
    UserMgr::GetInstance().AppendFriendChatMsg(from_uid, chat_msgs);

    auto iter = chat_item_added_.find(from_uid);
    if (iter != chat_item_added_.end()) {
        auto widget = ui->chat_user_list->itemWidget(iter->second);
        if (!widget) {
            qDebug() << __func__ << "widget is nullptr";
            return;
        }

        auto chat_user_widget = qobject_cast<ChatUserWidget*>(widget);
        if (!chat_user_widget) {
            qDebug() << __func__ << "chat_user_widget is nullptr";
            return;
        }

        chat_user_widget->updateLastMsg(chat_msgs);
        updateChatMsg(chat_msgs);
    } else {
        auto chat_user_widget = new ChatUserWidget;
        chat_user_widget->SetInfo(UserMgr::GetInstance().GetFriendById(from_uid));
        chat_user_widget->updateLastMsg(chat_msgs);
        auto item = new QListWidgetItem;
        item->setSizeHint(chat_user_widget->sizeHint());
        ui->chat_user_list->insertItem(0, item);
        ui->chat_user_list->setItemWidget(item, chat_user_widget);

        chat_item_added_[from_uid] = item;
    }
}

void ChatDialog::slot_jump_chat_item(const std::shared_ptr<UserInfo>& user_info) {
    qDebug() << "slot_jump_chat_item" << user_info->uid;

    auto iter = chat_item_added_.find(user_info->uid);
    if (iter != chat_item_added_.end()) {
        ui->chat_user_list->scrollToItem(iter->second);
    } else {
        // 创建新的 QListWidgetItem
        auto chat_user_widget = new ChatUserWidget;
        chat_user_widget->SetInfo(user_info);
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(chat_user_widget->sizeHint());
        ui->chat_user_list->insertItem(0, item);
        ui->chat_user_list->setItemWidget(item, chat_user_widget);

        chat_item_added_[user_info->uid] = item;
    }
    ui->side_chat_label->setSelected(ClickedLabel::State::kSelected);
    setSelectChatItem(user_info->uid);
    setSelectChatPage(user_info->uid);
    slot_side_chat();
}

void ChatDialog::slot_switch_apply_friend_page() {
    prev_page_ = ui->stackedWidget->currentWidget();
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_switch_friend_info_page(const std::shared_ptr<UserInfo>& user_info) {
    prev_page_ = ui->stackedWidget->currentWidget();
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->setUserInfo(user_info);
}

void ChatDialog::slot_chat_item_clicked(QListWidgetItem* item) {
    // 转为 Widget
    auto widget = ui->chat_user_list->itemWidget(item);
    if (!widget) {
        qDebug() << __func__ << "widget is nullptr";
        return;
    }

    auto chat_user_widget = qobject_cast<ChatUserWidget*>(widget);
    if (!chat_user_widget) {
        qDebug() << __func__ << "chat_user_widget is nullptr";
        return;
    }

    auto user_info = chat_user_widget->GetUserInfo();
    ui->chat_page->setUserInfo(user_info);
    current_chat_uid_ = user_info->uid;
}

void ChatDialog::slot_append_send_chat_msg(const std::shared_ptr<TextChatData>& chat_msg) {
    qDebug() << __func__ << "current_chat_uid_:" << current_chat_uid_;

    if (current_chat_uid_ == 0) {
        return;
    }

    auto iter = chat_item_added_.find(current_chat_uid_);
    if (iter == chat_item_added_.end()) {
        return;
    }

    auto widget = ui->chat_user_list->itemWidget(iter->second);
    if (!widget) {
        qDebug() << __func__ << "widget is nullptr";
        return;
    }

    auto chat_user_widget = qobject_cast<ChatUserWidget*>(widget);
    if (!chat_user_widget) {
        qDebug() << __func__ << "chat_user_widget is nullptr";
        return;
    }

    auto user_info = chat_user_widget->GetUserInfo();
    user_info->chat_msgs.push_back(chat_msg);
    UserMgr::GetInstance().AppendFriendChatMsg(current_chat_uid_, std::vector{chat_msg});
}
