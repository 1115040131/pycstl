#include "chat/client/util/timer_btn.h"

#include <QMouseEvent>

TimerBtn::TimerBtn(QWidget* parent) : QPushButton(parent) {
    timer_ = new QTimer(this);

    connect(timer_, &QTimer::timeout, [this]() {
        counter_--;
        if (counter_ <= 0) {
            timer_->stop();
            counter_ = kCounter;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(counter_));
    });
}

TimerBtn::~TimerBtn() { timer_->stop(); }

void TimerBtn::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        qDebug() << "MyButton was released!";
        this->setEnabled(false);
        this->setText(QString::number(counter_));
        timer_->start(1000);
        emit clicked();
    }
    // 调用基类的 mouseReleaseEvent 事件
    QPushButton::mouseReleaseEvent(e);
}
