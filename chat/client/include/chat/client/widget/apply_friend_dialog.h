#pragma once

#include <map>
#include <memory>
#include <queue>

#include <QDialog>

#include "chat/client/user_data.h"
#include "chat/client/util/clicked_label.h"
#include "chat/client/widget/friend_label.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ApplyFriendDialog;
}
QT_END_NAMESPACE

class ApplyFriendDialog : public QDialog {
    Q_OBJECT

public:
    explicit ApplyFriendDialog(QWidget* parent = nullptr);
    ~ApplyFriendDialog();

    void setSearchInfo(const std::shared_ptr<SearchInfo>& search_info);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // 创建一个新标签
    struct TipLabelInfo {
        ClickedLabel* label;
        int text_width;
        int text_height;
    };
    TipLabelInfo makeNewTipLabel(const QString& text, QWidget* parent = nullptr);

    // 初始化已有的标签
    void initTipLabels();

    // 添加备选标签显示并返回下一个位置
    QPoint addTipLabel(ClickedLabel* tip_label, const QPoint& cur_point, int text_width, int text_height);

    void resetLabels();

    // 给好友添加标签
    void addLabel(const QString& name);

private slots:
    // 展示更多标签
    void slot_show_more_label();

    // 输入 label 按下回车触发将标签加入展示栏
    void slot_label_enter();

    // 点击关闭, 移除展示栏好友标签
    void slot_remove_friend_label(const QString& name);

    // 通过点击 tip 实现增加和减少好友标签
    void slot_change_friend_label_by_tip(const QString& name, ClickedLabel::State state);

    // 输入框文本变化显示不同提示
    void slot_label_text_change(const QString& text);

    // 输入框输入完成
    void slot_label_edit_finished();

    // 输入标签显示提示框, 点击提示框内容后添加好友标签
    void slot_add_friend_label_by_click_tip(const QString& text);

    // 处理确认回调
    void slot_apply_sure();

    // 处理取消回调
    void slot_apply_cancel();

private:
    Ui::ApplyFriendDialog* ui;

    static constexpr int kTipOffset = 10;

    // 已经创建的标签
    std::map<QString, ClickedLabel*> add_labels_;
    std::vector<QString> add_label_keys;  // 记录加入时的顺序
    QPoint label_point_{2, 6};

    // 在输入框显示添加新好友标签
    std::map<QString, FriendLabel*> friend_labels_;
    std::vector<QString> friend_label_keys_;
    std::vector<QString> tip_data_;
    QPoint tip_cur_point_{kTipOffset, 5};

    std::shared_ptr<SearchInfo> search_info_ = nullptr;
};