#pragma once

#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui {
class FriendLabel;
}
QT_END_NAMESPACE

class FriendLabel : public QFrame {
    Q_OBJECT

public:
    explicit FriendLabel(QWidget* parent = nullptr);
    ~FriendLabel();

    void setText(const QString& text);

signals:
    void sig_close(const QString&);

private slots:
    void slot_close();

private:
    Ui::FriendLabel* ui;
    QString text_;
};