#pragma once

#include <QLabel>

class ClickedLabel : public QLabel {
    Q_OBJECT

public:
    ClickedLabel(QWidget* parent = nullptr);

    void setState(const QString& normal, const QString& hover, const QString& press, const QString& select,
                  const QString& select_hover, const QString& select_press);

    enum class State {
        kNormal,
        kSelected,
    };

    State getState() const { return state_; }
    void setSelected(State state);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void clicked(void);

protected:
    QString normal_;
    QString hover_;
    QString press_;

    QString select_;
    QString select_hover_;
    QString select_press_;

    State state_;
};