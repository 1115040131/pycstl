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

    const QString& text() const { return text_; }
    int width() const { return width_; }
    int height() const { return height_; }

signals:
    void sig_close(const QString&);

private slots:
    void slot_close();

private:
    Ui::FriendLabel* ui;
    QString text_;
    int width_;
    int height_;
};