#pragma once

#include "chat/client/util/list_item_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class AddUserItem;
}
QT_END_NAMESPACE

class AddUserItem : public ListItemBase {
public:
    explicit AddUserItem(QWidget* parent = nullptr);
    ~AddUserItem();

    QSize sizeHint() const override {
        return QSize(250, 70);  // 返回自定义的尺寸
    }

private:
    Ui::AddUserItem* ui;
};