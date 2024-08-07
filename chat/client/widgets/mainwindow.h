#pragma once

#include <QMainWindow>

#include "chat/client/widgets/login_dialog.h"
#include "chat/client/widgets/register_dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void SlotSwitchRegister();

private:
    Ui::MainWindow* ui;
    LoginDialog* login_dialog_;
    RegisterDialog* register_dialog_;
};
