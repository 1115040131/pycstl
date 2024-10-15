#include "chat/client/widget/chat_dialog.h"

#include "chat/client/widget/ui_chat_dialog.h"

ChatDialog::ChatDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ChatDialog) {
    ui->setupUi(this);
}

ChatDialog::~ChatDialog() { delete ui; }
