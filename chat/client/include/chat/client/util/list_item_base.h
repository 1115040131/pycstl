#pragma once

#include <QWidget>

class ListItemBase : public QWidget {
    Q_OBJECT

public:
    enum class Type {
        kChatUserItem,     // 聊天用户
        kContactUserItem,  // 联系人用户
        kSearchUserItem,   // 搜索到的用户
        kAddUserTipItem,   // 提示添加用户
        kInvalidItem,      // 不可点击条目
        kGroupTipItem,     // 分组提示条目
        kLineItem,         // 分割线
        kApplyFriendItem,  // 好友申请
    };

    explicit ListItemBase(QWidget* parent = nullptr);

    void SetItemType(Type type) { type_ = type; }

    Type GetItemType() const { return type_; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Type type_;
};