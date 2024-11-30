#include "chat/client/widget/friend_info_page.h"

#include "chat/client/widget/ui_friend_info_page.h"

FriendInfoPage::FriendInfoPage(QWidget* parent) : QDialog(parent), ui(new Ui::FriendInfoPage) {
    ui->setupUi(this);

    ui->msg_chat_btn->setState("normal", "hover", "press");
    ui->voice_chat_btn->setState("normal", "hover", "press");
    ui->video_chat_btn->setState("normal", "hover", "press");
}

FriendInfoPage::~FriendInfoPage() { delete ui; }

void FriendInfoPage::setUserInfo(const std::shared_ptr<UserInfo>& user_info) {
    user_info_ = user_info;

    QPixmap pixmap(user_info->icon);
    ui->icon_label->setPixmap(pixmap);
    ui->icon_label->setScaledContents(true);

    ui->name_label->setText(user_info->name);
    ui->nick_label->setText(user_info->nick);
    ui->back_label->setText(user_info->nick);
}

void FriendInfoPage::on_msg_chat_btn_clicked() {
    qDebug() << "on_msg_chat_btn_clicked";
    emit sig_jump_chat_item(user_info_);
}
