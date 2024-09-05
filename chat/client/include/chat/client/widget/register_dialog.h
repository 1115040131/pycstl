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
    // 注册按钮点击
    void on_get_code_btn_clicked();

    // 确认按钮点击
    void on_sure_btn_clicked();

    // http 请求完成
    void slot_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    // 初始化 http 回复处理
    void initHttpHandlers();

    // 显示提示
    void showTip(const QString& str, bool normal);

private:
    Ui::RegisterDialog* ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;
};