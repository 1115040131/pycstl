#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class FindFailDialog;
}
QT_END_NAMESPACE

class FindFailDialog : public QDialog {
    Q_OBJECT

public:
    explicit FindFailDialog(QWidget* parent = nullptr);
    ~FindFailDialog();

private slots:
    void on_fail_sure_btn_clicked();

private:
    Ui::FindFailDialog* ui;
};