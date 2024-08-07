#include "chat/client/widgets/register_dialog.h"

#include "chat/client/widgets/ui_register_dialog.h"

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
}

RegisterDialog::~RegisterDialog() { delete ui; }
