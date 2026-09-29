#pragma once

#include <QTextEdit>

import chat.client.util.bubble_frame;

class TextBubble : public BubbleFrame {
    Q_OBJECT

public:
    TextBubble(ChatRole role, const QString& text, QWidget* parent = nullptr);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void adjustTextHeight();

    void setPlainText(const QString& text);

    void initStyleSheet();

private:
    QTextEdit* text_edit_;
};