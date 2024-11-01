#include "chat/client/widget/chat_page.h"

#include <QPainter>
#include <QStyleOption>

#include "chat/client/util/chat_item_base.h"
#include "chat/client/util/picture_bubble.h"
#include "chat/client/util/text_bubble.h"
#include "chat/client/widget/ui_chat_page.h"

ChatPage::ChatPage(QWidget* parent) : QDialog(parent), ui(new Ui::ChatPage) {
    ui->setupUi(this);

    // 图标样式
    ui->emo_label->setState("normal", "hover", "press", "normal", "hover", "press");
    ui->file_label->setState("normal", "hover", "press", "normal", "hover", "press");
}

ChatPage::~ChatPage() { delete ui; }

// void ChatPage::paintEvent(QPaintEvent*) {
//     QStyleOption opt;
//     opt.initFrom(this);
//     QPainter painter(this);
//     style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
// }

void ChatPage::on_send_btn_clicked() {
    auto chat_edit = ui->chat_edit;
    ChatRole role = ChatRole::kSelf;
    QString user_name = QStringLiteral("PYC_Chat");
    QString user_icon = "chat/client/res/head_1.jpg";

    auto msg_list = chat_edit->getMsgList();
    for (const auto& msg : msg_list) {
        QWidget* bubble = nullptr;
        switch (msg.type) {
            case MsgInfo::Type::kText:
                bubble = new TextBubble(role, msg.content);
                break;
            case MsgInfo::Type::kImage:
                bubble = new PictureBubble(role, QPixmap(msg.pixmap));
                break;
            case MsgInfo::Type::kFile:
                // TODO: 发送文件
                break;
            default:
                break;
        }

        if (bubble) {
            auto chat_item = new ChatItemBase(role);
            chat_item->setUserName(user_name);
            chat_item->setUserIcon(user_icon);
            chat_item->setWidget(bubble);
            ui->chat_data_list->appendChatItem(chat_item);
        }
    }
}