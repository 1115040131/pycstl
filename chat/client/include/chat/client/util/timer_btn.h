#pragma once

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton {
public:
    TimerBtn(QWidget* parent = nullptr);
    ~TimerBtn();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    static constexpr int kCounter = 3;

    QTimer* timer_;
    int counter_{kCounter};
};