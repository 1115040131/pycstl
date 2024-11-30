#include "chat/client/widget/chat_page.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>

#include "chat/client/tcp_mgr.h"
#include "chat/client/user_mgr.h"
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

void ChatPage::setUserInfo(const std::shared_ptr<UserInfo>& user_info) {
    user_info_ = user_info;

    ui->title_label->setText(user_info->name);
    ui->chat_data_list->removeAllItem();
    for (const auto& chat_msg : user_info->chat_msgs) {
        ui->chat_data_list->appendChatMsg(chat_msg);
    }
}

void ChatPage::appendChatMsg(const std::shared_ptr<TextChatData>& chat_msg) {
    ui->chat_data_list->appendChatMsg(chat_msg);
}

void ChatPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ChatPage::on_send_btn_clicked() {
    if (!user_info_) {
        qDebug() << __func__ << "user_info_ is nullptr";
        return;
    }

    ChatRole role = ChatRole::kSelf;
    auto msg_list = ui->chat_edit->getMsgList();

    int text_size = 0;
    QJsonObject root;
    QJsonArray text_array;

    for (const auto& msg : msg_list) {
        if (msg.content.length() > 1024) {
            continue;
        }

        QWidget* bubble = nullptr;
        switch (msg.type) {
            case MsgInfo::Type::kText: {
                auto msg_id = QUuid::createUuid().toString();
                bubble = new TextBubble(role, msg.content);
                if (text_size + msg.content.size() > 1024) {
                    // 发送并清空之前累积的文本
                    root["from_uid"] = UserMgr::GetInstance().GetUid();
                    root["to_uid"] = user_info_->uid;
                    root["text_array"] = text_array;
                    QJsonDocument doc(root);
                    emit TcpMgr::GetInstance().sig_send_data(ReqId::kTextChatMsgReq,
                                                             doc.toJson(QJsonDocument::Compact));

                    text_size = 0;
                    root = QJsonObject();
                    text_array = QJsonArray();
                }

                text_size += msg.content.size();
                QJsonObject text;
                text["msg_id"] = msg_id;
                text["content"] = msg.content;
                text_array.append(text);

                auto chat_msg = std::make_shared<TextChatData>(msg_id, msg.content,
                                                               UserMgr::GetInstance().GetUid(), user_info_->uid);
                emit sig_append_chat_msg(chat_msg);
            } break;
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
            chat_item->setUserName(UserMgr::GetInstance().GetName());
            chat_item->setUserIcon(UserMgr::GetInstance().GetIcon());
            chat_item->setWidget(bubble);
            ui->chat_data_list->appendChatItem(chat_item);
        }
    }

    // 发送剩余信息
    qDebug() << __func__ << "text_size: " << text_size << " text_array: " << text_array;
    if (text_size > 0) {
        root["from_uid"] = UserMgr::GetInstance().GetUid();
        root["to_uid"] = user_info_->uid;
        root["text_array"] = text_array;
        QJsonDocument doc(root);
        emit TcpMgr::GetInstance().sig_send_data(ReqId::kTextChatMsgReq, doc.toJson(QJsonDocument::Compact));
    }
}