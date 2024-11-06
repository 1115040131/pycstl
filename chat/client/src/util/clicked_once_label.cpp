#include "chat/client/util/clicked_once_label.h"

#include <QMouseEvent>

ClickedOnceLabel::ClickedOnceLabel(QWidget* parent) : QLabel(parent) { setCursor(Qt::PointingHandCursor); }

void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
        return;
    }
    QLabel::mouseReleaseEvent(event);
}
