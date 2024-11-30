#pragma once

#include <QPushButton>

class ClickedBtn : public QPushButton {
    Q_OBJECT

public:
    ClickedBtn(QWidget* parent = nullptr);

    void setState(const QString& normal, const QString& hover, const QString& press);

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString normal_;
    QString hover_;
    QString press_;
};