#pragma once

#include <QDialog>
#include <memory>

#include "chat/client/user_data.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class FindSuccessDialog;
}
QT_END_NAMESPACE

class FindSuccessDialog : public QDialog {
    Q_OBJECT

public:
    explicit FindSuccessDialog(QWidget* parent = nullptr);
    ~FindSuccessDialog();

    void setSearchInfo(const std::shared_ptr<SearchInfo>& search_info);

private slots:
    // 添加好友界面弹出
    void on_add_friend_btn_clicked();

private:
    Ui::FindSuccessDialog* ui;

    std::shared_ptr<SearchInfo> search_info_ = nullptr;
};