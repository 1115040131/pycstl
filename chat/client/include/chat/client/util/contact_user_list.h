#pragma once

#include <QListWidget>

#include "chat/client/user_data.h"

class ContactUserItem;

class ContactUserList : public QListWidget {
    Q_OBJECT

public:
    ContactUserList(QWidget* parent = nullptr);

    void showRedPoint(bool show);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void addContactUserList();

    void addAuthFriendItem(const std::shared_ptr<AuthInfo>& auth_info);

signals:
    void sig_loading_contact_user();

    void sig_switch_apply_friend_page();

    void sig_switch_friend_info_page(const std::shared_ptr<UserInfo>& user_info);

public slots:
    void slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_info);

    void slot_add_auth_friend(const std::shared_ptr<AuthInfo>& auth_info);

private slots:
    void slot_item_clicked(QListWidgetItem* item);

private:
    ContactUserItem* add_friend_item_;
    QListWidgetItem* group_item_;
};