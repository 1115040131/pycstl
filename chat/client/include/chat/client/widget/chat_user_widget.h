#pragma once

#include "chat/client/user_data.h"
#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatUserWidget;
}
QT_END_NAMESPACE

class ChatUserWidget : public ListItemBase {
public:
    explicit ChatUserWidget(QWidget* parent = nullptr);
    ~ChatUserWidget();

    QSize sizeHint() const override {
        return QSize(250, 70);  // 返回自定义的尺寸
    }

    void SetInfo(const QString& name, const QString& head, const QString& msg);

    void SetInfo(const std::shared_ptr<UserInfo>& user_info);

private:
    Ui::ChatUserWidget* ui;
    std::shared_ptr<UserInfo> user_info_;
};