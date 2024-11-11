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

private:
    void addNewApply(const AddFriendApply& apply);

    void loadApplyList();

signals:
    void sig_show_search(bool);

private slots:
    void slot_auth_rsp(const AuthRsp& auth_rsp);

private:
    Ui::ApplyFriendPage* ui;

    std::unordered_map<int, ApplyFriendItem*> unauth_items_;
};