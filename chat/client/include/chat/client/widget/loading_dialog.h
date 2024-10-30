#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoadingDialog;
}
QT_END_NAMESPACE

class LoadingDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadingDialog(QWidget* parent = nullptr);
    ~LoadingDialog();

private:
    Ui::LoadingDialog* ui;
};