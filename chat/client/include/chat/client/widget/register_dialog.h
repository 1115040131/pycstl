#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class RegisterDialog;
}
QT_END_NAMESPACE

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget* parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_btn_clicked();

private:
    void showTip(const QString& str, bool normal);

private:
    Ui::RegisterDialog* ui;
};