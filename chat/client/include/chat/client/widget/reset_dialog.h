#pragma once

#include <QDialog>
#include <QJsonObject>

#include "chat/client/define.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ResetDialog;
}
QT_END_NAMESPACE

class ResetDialog : public QDialog {
    Q_OBJECT

public:
    explicit ResetDialog(QWidget* parent = nullptr);
    ~ResetDialog();

signals:
    void switchLogin();

private slots:
    // 获取(验证码)按钮点击
    void on_verify_btn_clicked();

    // 确认(修改密码)按钮点击
    void on_sure_btn_clicked();

    // 取消(修改密码)按钮点击
    void on_return_btn_clicked();

    // http 请求完成
    void slot_reset_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    // 初始化 http 回复处理
    void initHttpHandlers();

private:
    Ui::ResetDialog* ui;
    std::map<ReqId, std::function<void(const QJsonObject&)>> handlers_;
};