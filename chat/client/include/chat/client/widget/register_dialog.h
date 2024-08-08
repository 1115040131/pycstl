#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QMap>
#include <functional>

#include "chat/client/define.h"

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

    void slot_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    void initHttpHandlers();

    void showTip(const QString& str, bool normal);

private:
    Ui::RegisterDialog* ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;
};