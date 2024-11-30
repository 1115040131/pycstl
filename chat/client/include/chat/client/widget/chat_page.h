#pragma once

#include <QDialog>

#include "chat/client/define.h"
#include "chat/client/user_data.h"

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

    void setUserInfo(const std::shared_ptr<UserInfo>& user_info);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage* ui;

    std::shared_ptr<UserInfo> user_info_;
};