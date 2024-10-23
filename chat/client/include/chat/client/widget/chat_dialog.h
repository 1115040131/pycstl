#pragma once

#include <QDialog>

#include "chat/client/define.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatDialog;
}
QT_END_NAMESPACE

class ChatDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChatDialog(QWidget* parent = nullptr);
    ~ChatDialog();

private:
    void ShowSearch(bool is_search);

    // 测试函数, 添加用户列表
    void addChatUserList();

private:
    Ui::ChatDialog* ui;

    enum class UIMode {
        kSearchMode,
        kChatMode,
        kContactMode,
    };
    UIMode state_ = UIMode::kChatMode;
    UIMode mode_ = UIMode::kChatMode;

    bool is_loading_ = false;
};