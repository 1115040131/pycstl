#include "chat/client/util/list_item_base.h"

#include <QPainter>
#include <QStyleOption>

ListItemBase::ListItemBase(QWidget* parent) : QWidget(parent) {}

// void ListItemBase::paintEvent(QPaintEvent*) {
//     QStyleOption opt;
//     opt.initFrom(this);
//     QPainter painter(this);
//     style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
// }