#pragma once

#include <QDialog>
#include <unordered_map>

#include "chat/client/user_data.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ApplyFriendPage;
}
QT_END_NAMESPACE

class ApplyFriendItem;

class ApplyFriendPage : public QDialog {
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget* parent = nullptr);
    ~ApplyFriendPage();

    void addNewApply(const std::shared_ptr<ApplyInfo>& apply_info);

private:
    void loadApplyList();

signals:
    void sig_show_search(bool);

private slots:
    // 弹出同意好友申请对话框
    void slot_auth_friend(const std::shared_ptr<ApplyInfo>& apply_info);

    void slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_info);

private:
    Ui::ApplyFriendPage* ui;

    std::unordered_map<int, ApplyFriendItem*> unauth_items_;
};