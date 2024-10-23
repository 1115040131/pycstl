#pragma once

#include <QLineEdit>

class CustomizeEdit : public QLineEdit {
    Q_OBJECT

public:
    CustomizeEdit(QWidget* parent = nullptr);

    void SetMaxLength(int max_length) { max_length_ = max_length; }

protected:
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    void limitTextLength(const QString& text);

signals:
    void sig_focus_out();

private:
    int max_length_ = 15;
};