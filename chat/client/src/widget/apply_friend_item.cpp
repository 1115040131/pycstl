#include "chat/client/widget/apply_friend_item.h"

#include "chat/client/widget/ui_apply_friend_item.h"

ApplyFriendItem::ApplyFriendItem(QWidget* parent) : ListItemBase(parent), ui(new Ui::ApplyFriendItem) {
    ui->setupUi(this);
    SetItemType(Type::kApplyFriendItem);
    showAddBtn(false);
    connect(ui->add_btn, &ClickedBtn::clicked, [this]() { emit this->sig_auth_friend(apply_info_); });
}

ApplyFriendItem::~ApplyFriendItem() { delete ui; }

void ApplyFriendItem::setInfo(const ApplyInfo& apply_info) {
    apply_info_ = apply_info;

    QPixmap pixmap(apply_info_.icon);
    ui->icon_label->setPixmap(pixmap);
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(apply_info_.name);
    ui->user_chat_label->setText(apply_info_.desc);
}

void ApplyFriendItem::showAddBtn(bool show) {
    ui->add_btn->setVisible(show);
    ui->already_add_label->setVisible(!show);
    added_ = !show;
}
