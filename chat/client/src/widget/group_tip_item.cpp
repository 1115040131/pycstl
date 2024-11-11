#include "chat/client/widget/group_tip_item.h"

#include "chat/client/widget/ui_group_tip_item.h"

GroupTipItem::GroupTipItem(QWidget* parent) : ListItemBase(parent), ui(new Ui::GroupTipItem) {
    ui->setupUi(this);
    SetItemType(Type::kGroupTipItem);
}

GroupTipItem::~GroupTipItem() { delete ui; }

void GroupTipItem::setGroupTip(const QString& tip) { ui->label->setText(tip); }
