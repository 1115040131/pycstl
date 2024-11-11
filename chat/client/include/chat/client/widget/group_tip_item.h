#pragma once

#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class GroupTipItem;
}
QT_END_NAMESPACE

class GroupTipItem : public ListItemBase {
public:
    explicit GroupTipItem(QWidget* parent = nullptr);
    ~GroupTipItem();

    QSize sizeHint() const override { return QSize(250, 25); }

    void setGroupTip(const QString& tip);

private:
    Ui::GroupTipItem* ui;
};