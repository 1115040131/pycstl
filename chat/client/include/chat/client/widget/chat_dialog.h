#pragma once

#include <QAction>
#include <QDialog>
#include <QListWidgetItem>

#include "chat/client/user_data.h"

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

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void ShowSearch(bool is_search);

    // 添加聊天列表
    void addChatUserList();

    // 添加好友列表
    void addContactUserList();

    // 添加需要联动的标签
    void addLabelGroup(StateWidget* label);

    // 设置联动标签状态
    void clearLabelState(StateWidget* select_label);

    // 处理全局鼠标点击事件, 判断是否需要清空搜索框
    void handleGlobalMousePress(QMouseEvent* event);

    // 添加好友
    void addAuthFriend(const std::shared_ptr<AuthInfo>& auth_info);

    // 设置当前聊天窗口
    void setSelectChatItem(int uid);

    void setSelectChatPage(int uid);

private slots:
    // 搜索框文本变化
    void slot_search_text_changed(const QString& text);

    // 加载更多聊天用户
    void slot_loading_chat_user();

    // 加载更多联系人
    void slot_loading_contact_user();

    // 点击侧边栏
    void slot_side_chat();
    void slot_side_contact();

    // 收到好友请求
    void slot_friend_apply(std::shared_ptr<ApplyInfo> apply);

    // 同意好友请求
    void slot_auth_rsp(const std::shared_ptr<AuthInfo>& auth_info);

    // 收到同意好友请求
    void slot_add_auth_friend(const std::shared_ptr<AuthInfo>& auth_info);

    // 跳转到聊天界面
    void slot_jump_chat_item(const std::shared_ptr<UserInfo>& user_info);

    // 跳转申请好友界面
    void slot_switch_apply_friend_page();

    // 跳转好友信息界面
    void slot_switch_friend_info_page(const std::shared_ptr<UserInfo>& user_info);

    // 点击聊天列表
    void slot_chat_item_clicked(QListWidgetItem* item);

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

    int current_chat_uid_ = 0;
    std::unordered_map<int, QListWidgetItem*> chat_item_added_;

    QWidget* prev_page_ = nullptr;
};