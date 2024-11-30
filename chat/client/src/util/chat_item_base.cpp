#include "chat/client/util/chat_item_base.h"

#include <QGridLayout>

ChatItemBase::ChatItemBase(ChatRole role, QWidget* parent) : QWidget(parent), role_(role) {
    name_label_ = new QLabel();
    name_label_->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    name_label_->setFont(font);
    name_label_->setFixedHeight(20);

    icon_label_ = new QLabel();
    icon_label_->setScaledContents(true);
    icon_label_->setFixedSize(42, 42);

    bubble_ = new QWidget();

    QGridLayout* grid_layout = new QGridLayout();
    grid_layout->setVerticalSpacing(3);
    grid_layout->setHorizontalSpacing(3);
    grid_layout->setContentsMargins(3, 3, 3, 3);

    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    if (role == ChatRole::kSelf) {
        name_label_->setContentsMargins(0, 0, 8, 0);
        name_label_->setAlignment(Qt::AlignRight);
        grid_layout->addWidget(name_label_, 0, 1, 1, 1);
        grid_layout->addWidget(icon_label_, 0, 2, 2, 1, Qt::AlignTop);
        grid_layout->addWidget(bubble_, 1, 1, 1, 1);
        grid_layout->addItem(spacer, 1, 0, 1, 1);
        grid_layout->setColumnStretch(0, 2);
        grid_layout->setColumnStretch(1, 3);
    } else {
        name_label_->setContentsMargins(8, 0, 0, 0);
        name_label_->setAlignment(Qt::AlignLeft);
        grid_layout->addWidget(name_label_, 0, 1, 1, 1);
        grid_layout->addWidget(icon_label_, 0, 0, 2, 1, Qt::AlignTop);
        grid_layout->addWidget(bubble_, 1, 1, 1, 1);
        grid_layout->addItem(spacer, 2, 2, 1, 1);
        grid_layout->setColumnStretch(1, 3);
        grid_layout->setColumnStretch(2, 2);
    }

    setLayout(grid_layout);
}

void ChatItemBase::setUserName(const QString& name) { name_label_->setText(name); }

void ChatItemBase::setUserIcon(const QPixmap& icon) { icon_label_->setPixmap(icon); }

void ChatItemBase::setWidget(QWidget* w) {
    auto grid_layout = qobject_cast<QGridLayout*>(layout());
    grid_layout->replaceWidget(bubble_, w);
    delete bubble_;
    bubble_ = w;
}
