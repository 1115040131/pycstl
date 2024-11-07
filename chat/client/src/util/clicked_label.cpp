#include "chat/client/util/clicked_label.h"

#include <QMouseEvent>

#include "chat/client/api.h"

ClickedLabel::ClickedLabel(QWidget* parent) : QLabel(parent), state_(State::kNormal) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

void ClickedLabel::setState(const QString& normal, const QString& hover, const QString& press,
                            const QString& select, const QString& select_hover, const QString& select_press) {
    normal_ = normal;
    hover_ = hover;
    press_ = press;

    select_ = select;
    select_hover_ = select_hover;
    select_press_ = select_press;

    setProperty("state", normal);
    repolish(this);
}

void ClickedLabel::setSelected(State state) {
    state_ = state;
    if (state == State::kSelected) {
        setProperty("state", select_);
    } else {
        setProperty("state", normal_);
    }
    repolish(this);
    update();
}

void ClickedLabel::enterEvent(QEnterEvent* event) {
    if (state_ == State::kNormal) {
        qDebug() << "enter, changed to normal hover: " << hover_;
        setProperty("state", hover_);
    } else {
        qDebug() << "enter, changed to select hover: " << select_hover_;
        setProperty("state", select_hover_);
    }
    repolish(this);
    update();

    QLabel::enterEvent(event);
}

void ClickedLabel::leaveEvent(QEvent* event) {
    if (state_ == State::kNormal) {
        qDebug() << "leave, changed to normal: " << normal_;
        setProperty("state", normal_);
    } else {
        qDebug() << "leave, changed to select: " << select_;
        setProperty("state", select_);
    }
    repolish(this);
    update();

    QLabel::leaveEvent(event);
}

void ClickedLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (state_ == State::kNormal) {
            qDebug() << "clicked, changed to normal press:" << press_;
            setProperty("state", press_);
        } else {
            qDebug() << "clicked, changed to select press:" << select_press_;
            setProperty("state", select_press_);
        }
        repolish(this);
        update();
        return;
    }
    QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (state_ == State::kNormal) {
            qDebug() << "release, changed to select hover:" << select_hover_;
            state_ = State::kSelected;
            setProperty("state", select_hover_);
        } else {
            qDebug() << "release, changed to normal hover:" << hover_;
            state_ = State::kNormal;
            setProperty("state", hover_);
        }
        repolish(this);
        update();
        emit clicked(this->text(), state_);
        return;
    }
    QLabel::mouseReleaseEvent(event);
}
