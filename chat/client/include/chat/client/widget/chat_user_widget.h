#pragma once

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

private:
    Ui::ChatUserWidget* ui;
    QString name_;
    QString head_;
    QString msg_;
};