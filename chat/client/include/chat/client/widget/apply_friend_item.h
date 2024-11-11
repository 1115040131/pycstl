#pragma once

#include "chat/client/user_data.h"
#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ApplyFriendItem;
}
QT_END_NAMESPACE

class ApplyFriendItem : public ListItemBase {
    Q_OBJECT
public:
    explicit ApplyFriendItem(QWidget* parent = nullptr);
    ~ApplyFriendItem();

    QSize sizeHint() const override { return {250, 80}; }

    int getUid() const { return apply_info_.uid; }

    void setInfo(const ApplyInfo& apply_info);

    void showAddBtn(bool show);

signals:
    void sig_auth_friend(const ApplyInfo& apply_info);

private:
    Ui::ApplyFriendItem* ui;

    bool added_;
    ApplyInfo apply_info_;
};