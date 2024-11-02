#include "chat/client/util/state_widget.h"

#include <QVBoxLayout>

StateWidget::StateWidget(QWidget* parent) : ClickedLabel(parent) { AddRedPoint(); }

void StateWidget::AddRedPoint() {
    red_point_ = new QLabel();
    red_point_->setObjectName("red_point");
    QVBoxLayout* layout = new QVBoxLayout();
    red_point_->setAlignment(Qt::AlignCenter);
    layout->addWidget(red_point_);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    // red_point_->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show) { red_point_->setVisible(show); }
