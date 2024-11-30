#pragma once

#include <QWidget>

#include "chat/client/user_data.h"

class QScrollArea;

class ChatView : public QWidget {
    Q_OBJECT

public:
    ChatView(QWidget* parent = nullptr);

    void appendChatItem(QWidget* item);
    void prependChatItem(QWidget* item);
    void insertChatItem(QWidget* position, QWidget* item);
    void removeAllItem();
    void appendChatMsg(const std::shared_ptr<TextChatData>& chat_msg);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onVerticalScrollBarMoved(int min, int max);

private:
    QScrollArea* scroll_area_;
    bool is_appended = false;
};