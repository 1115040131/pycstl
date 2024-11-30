#include "chat/client/widget/chat_user_widget.h"

#include "chat/client/widget/ui_chat_user_widget.h"

ChatUserWidget::ChatUserWidget(QWidget* parent) : ListItemBase(parent), ui(new Ui::ChatUserWidget) {
    ui->setupUi(this);
    SetItemType(Type::kChatUserItem);
}

ChatUserWidget::~ChatUserWidget() { delete ui; }

void ChatUserWidget::SetInfo(const std::shared_ptr<UserInfo>& user_info) {
    user_info_ = user_info;

    // 加载图片
    QPixmap pixmap(user_info->icon);
    ui->icon_label->setPixmap(pixmap);
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(user_info->name);
    ui->user_chat_label->setText(user_info->last_msg);
}

void ChatUserWidget::SetInfo(const std::shared_ptr<FriendInfo>& friend_info) {
    user_info_ = std::make_shared<UserInfo>(UserInfo::FromFriendInfo(*friend_info));

    // 加载图片
    QPixmap pixmap(user_info_->icon);
    ui->icon_label->setPixmap(pixmap);
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(user_info_->name);
    ui->user_chat_label->setText(user_info_->last_msg);
}

void ChatUserWidget::updateLastMsg(const std::vector<std::shared_ptr<TextChatData>>& chat_msgs) {
    user_info_->chat_msgs.insert(user_info_->chat_msgs.end(), chat_msgs.begin(), chat_msgs.end());

    user_info_->last_msg = chat_msgs.back()->msg_content;
    ui->user_chat_label->setText(user_info_->last_msg);
}

void ChatUserWidget::paintEvent(QPaintEvent* event) { return QWidget::paintEvent(event); }