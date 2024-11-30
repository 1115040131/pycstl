#pragma once

#include <QLabel>

class ClickedOnceLabel : public QLabel {
    Q_OBJECT

public:
    ClickedOnceLabel(QWidget* parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void clicked(const QString& text);
};