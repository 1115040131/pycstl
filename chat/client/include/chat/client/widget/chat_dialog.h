#pragma once

#include <QAction>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatDialog;
}
QT_END_NAMESPACE

class StateWidget;

class ChatDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChatDialog(QWidget* parent = nullptr);
    ~ChatDialog();

private:
    void ShowSearch(bool is_search);

    // 测试函数, 添加用户列表
    void addChatUserList();

    // 添加需要联动的标签
    void addLabelGroup(StateWidget* label);

    // 设置联动标签状态
    void clearLabelState(StateWidget* select_label);

private slots:
    void slot_search_text_changed(const QString& text);

    void slot_loading_chat_user();

    void slot_side_chat();

    void slot_side_contact();

private:
    Ui::ChatDialog* ui;

    enum class UIMode {
        kSearchMode,
        kChatMode,
        kContactMode,
    };
    UIMode state_ = UIMode::kChatMode;
    UIMode mode_ = UIMode::kChatMode;

    QAction* search_action_ = nullptr;  // 搜索图标
    QAction* clear_action_ = nullptr;   // 清楚图标

    bool is_loading_ = false;

    std::vector<StateWidget*> labels_;
};