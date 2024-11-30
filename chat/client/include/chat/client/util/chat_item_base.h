#pragma once

#include <QLabel>
#include <QWidget>

#include "chat/client/define.h"

/// @brief 气泡聊天框基类
class ChatItemBase : public QWidget {
    Q_OBJECT

public:
    explicit ChatItemBase(ChatRole role, QWidget* parent = nullptr);

    void setUserName(const QString& name);
    void setUserIcon(const QPixmap& icon);
    void setWidget(QWidget* w);

    ChatRole getRole() const { return role_; }

private:
    ChatRole role_;
    QLabel* name_label_;
    QLabel* icon_label_;
    QWidget* bubble_;
};