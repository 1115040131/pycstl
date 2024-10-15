#pragma once

#include <QDialog>

#include "chat/client/define.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatDialog;
}
QT_END_NAMESPACE

class ChatDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChatDialog(QWidget* parent = nullptr);
    ~ChatDialog();

private:
    Ui::ChatDialog* ui;
};