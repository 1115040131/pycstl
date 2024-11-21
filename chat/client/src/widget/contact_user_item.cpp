#include "chat/client/widget/contact_user_item.h"

#include "chat/client/widget/ui_contact_user_item.h"

ContactUserItem::ContactUserItem(QWidget* parent) : ListItemBase(parent), ui(new Ui::ContactUserItem) {
    ui->setupUi(this);
    SetItemType(Type::kContactUserItem);

    ui->red_point->raise();
    showRedPoint(false);
}

ContactUserItem::~ContactUserItem() { delete ui; }

void ContactUserItem::setInfo(const AuthInfo& auth_info) {
    user_info_ = UserInfo::FromAuthInfo(auth_info);
    setUiData();
}

void ContactUserItem::setInfo(const AuthRsp& auth_rsp) {
    user_info_ = UserInfo::FromAuthRsp(auth_rsp);
    setUiData();
}

void ContactUserItem::setInfo(int uid, const QString& name, const QString& icon) {
    user_info_.uid = uid;
    user_info_.name = name;
    user_info_.icon = icon;
    setUiData();
}

void ContactUserItem::showRedPoint(bool show) { ui->red_point->setVisible(show); }

void ContactUserItem::setUiData() {
    // 加载图片
    QPixmap pixmap(user_info_.icon);
    ui->icon_label->setPixmap(pixmap);
    ui->icon_label->setScaledContents(true);

    // 设置名字
    ui->user_name_label->setText(user_info_.name);
}
