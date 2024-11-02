#include "chat/client/widget/chat_user_widget.h"

#include "chat/client/widget/ui_chat_user_widget.h"

ChatUserWidget::ChatUserWidget(QWidget* parent) : ListItemBase(parent), ui(new Ui::ChatUserWidget) {
    ui->setupUi(this);
    SetItemType(Type::kChatUserItem);
}

ChatUserWidget::~ChatUserWidget() { delete ui; }

void ChatUserWidget::SetInfo(const QString& name, const QString& head, const QString& msg) {
    name_ = name;
    head_ = head;
    msg_ = msg;

    // 加载图片
    QPixmap pixmap(head_);

    // 设置图片自动缩放
    ui->icon_label->setPixmap(
        pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(name_);
    ui->user_chat_label->setText(msg_);
}
