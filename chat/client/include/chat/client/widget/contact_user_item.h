#pragma once

#include "chat/client/user_data.h"
#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ContactUserItem;
}
QT_END_NAMESPACE

class ContactUserItem : public ListItemBase {
    Q_OBJECT

public:
    explicit ContactUserItem(QWidget* parent = nullptr);
    ~ContactUserItem();

    QSize sizeHint() const override { return QSize(250, 70); }

    const std::shared_ptr<UserInfo>& getUserInfo() const { return user_info_; }

    void setInfo(const std::shared_ptr<AuthInfo>& auth_info);
    void setInfo(const std::shared_ptr<FriendInfo>& friend_info);
    void setInfo(int uid, const QString& name, const QString& icon);

    void showRedPoint(bool show);

private:
    void setUiData();

private:
    Ui::ContactUserItem* ui;
    std::shared_ptr<UserInfo> user_info_;
};