#pragma once

#include <QDialog>

#include "chat/client/user_data.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class FriendInfoPage;
}
QT_END_NAMESPACE

class FriendInfoPage : public QDialog {
    Q_OBJECT
public:
    explicit FriendInfoPage(QWidget* parent = nullptr);
    ~FriendInfoPage();

    void setUserInfo(const std::shared_ptr<UserInfo>& user_info);

signals:
    void sig_jump_chat_item(const std::shared_ptr<UserInfo>& user_info);

private slots:
    void on_msg_chat_btn_clicked();

private:
    Ui::FriendInfoPage* ui;

    std::shared_ptr<UserInfo> user_info_;
};