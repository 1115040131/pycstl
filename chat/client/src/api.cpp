#include "chat/client/api.h"

#include <QStyle>

/**
 * @brief 刷新 qss
 */
void Repolish(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
}