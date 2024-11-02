#include "chat/client/widget/add_user_item.h"

#include "chat/client/widget/ui_add_user_item.h"

AddUserItem::AddUserItem(QWidget* parent) : ListItemBase(parent), ui(new Ui::AddUserItem) {
    ui->setupUi(this);
    SetItemType(Type::kAddUserTipItem);
}

AddUserItem::~AddUserItem() { delete ui; }