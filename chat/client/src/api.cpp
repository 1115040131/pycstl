#include "chat/client/api.h"

#include <QStyle>

void repolish(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
}

QString xorString(const QString& input) {
    QString result = input;
    size_t length = input.length() % 255;
    for (auto& c : result) {
        // 对每个字符异或
        // 这里假设字符都是 ASCII, 因此直接转换为 QChar
        c = QChar(static_cast<ushort>(c.unicode()) ^ static_cast<ushort>(length));
    }
    return result;
}