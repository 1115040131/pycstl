#pragma once

#include "chat/client/util/clicked_label.h"

class StateWidget : public ClickedLabel {
    Q_OBJECT
public:
    explicit StateWidget(QWidget* parent = nullptr);

    void AddRedPoint();
    void ShowRedPoint(bool show);

private:
    QLabel* red_point_;
};
