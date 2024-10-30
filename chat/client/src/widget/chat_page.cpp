#include "chat/client/widget/chat_page.h"

#include <QPainter>
#include <QStyleOption>

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
