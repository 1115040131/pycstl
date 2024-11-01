#pragma once

#include <QDialog>

#include "chat/client/define.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatPage;
}
QT_END_NAMESPACE

class ChatPage : public QDialog {
    Q_OBJECT

public:
    explicit ChatPage(QWidget* parent = nullptr);
    ~ChatPage();

protected:
    // virtual void paintEvent(QPaintEvent* event) override;

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage* ui;
};