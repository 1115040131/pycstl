#include "chat/client/util/clicked_btn.h"

#include <QMouseEvent>

#include "chat/client/api.h"

ClickedBtn::ClickedBtn(QWidget* parent) : QPushButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setState("normal", "hover", "press");
}

void ClickedBtn::setState(const QString& normal, const QString& hover, const QString& press) {
    normal_ = normal;
    hover_ = hover;
    press_ = press;

    setProperty("state", normal);
    repolish(this);
}

void ClickedBtn::enterEvent(QEnterEvent* event) {
    setProperty("state", hover_);
    repolish(this);

    QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent* event) {
    setProperty("state", normal_);
    repolish(this);

    QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent* event) {
    setProperty("state", press_);
    repolish(this);

    QPushButton::mousePressEvent(event);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent* event) {
    setProperty("state", hover_);
    repolish(this);

    QPushButton::mouseReleaseEvent(event);
}