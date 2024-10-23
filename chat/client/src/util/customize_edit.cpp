#include "chat/client/util/customize_edit.h"

CustomizeEdit::CustomizeEdit(QWidget* parent) : QLineEdit(parent) {
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    emit sig_focus_out();
}

void CustomizeEdit::limitTextLength(const QString& text) {
    if (max_length_ <= 0) {
        return;
    }

    QByteArray bytes = text.toUtf8();

    if (bytes.size() > max_length_) {
        int len = 0;
        int i = 0;

        for (; i < bytes.size() && len < max_length_; ++i) {
            if (bytes[i] & 0x80) {
                len += 3;
            } else {
                len += 1;
            }
        }

        setText(text.left(i));
    }
}
