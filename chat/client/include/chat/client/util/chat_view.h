#pragma once

#include <QWidget>

class QScrollArea;

class ChatView : public QWidget {
    Q_OBJECT

public:
    ChatView(QWidget* parent = nullptr);

    void appendChatItem(QWidget* item);
    void prependChatItem(QWidget* item);
    void insertChatItem(QWidget* position, QWidget* item);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initStyleSheet();

private slots:
    void onVerticalScrollBarMoved(int min, int max);

private:
    QScrollArea* scroll_area_;
    bool is_appended = false;
};