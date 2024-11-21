#pragma once

#include <QListWidget>

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

signals:
    void sig_loading_contact_user();

    void sig_switch_apply_friend_page();

    void sig_switch_friend_info_page();

private slots:
    void slot_item_clicked(QListWidgetItem* item);

    // void slot_add_auth_friend();

    // void slot_auth_rsp();

private:
    ContactUserItem* add_friend_item_;
    QListWidgetItem* group_item_;
};