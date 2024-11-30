#pragma once

#include <QListWidget>

class ChatUserList : public QListWidget {
    Q_OBJECT

public:
    ChatUserList(QWidget* parent = nullptr);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void sig_loading_chat_user();
};