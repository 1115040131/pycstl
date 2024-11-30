#pragma once

#include "chat/client/user_data.h"
#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatUserWidget;
}
QT_END_NAMESPACE

class ChatUserWidget : public ListItemBase {
    Q_OBJECT

public:
    explicit ChatUserWidget(QWidget* parent = nullptr);
    ~ChatUserWidget();

    QSize sizeHint() const override {
        return QSize(250, 70);  // 返回自定义的尺寸
    }

    void SetInfo(const std::shared_ptr<UserInfo>& user_info);
    void SetInfo(const std::shared_ptr<FriendInfo>& friend_info);

    const std::shared_ptr<UserInfo>& GetUserInfo() const { return user_info_; }

    void updateLastMsg(const std::vector<std::shared_ptr<TextChatData>>& chat_msgs);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Ui::ChatUserWidget* ui;
    std::shared_ptr<UserInfo> user_info_;
};