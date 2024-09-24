#pragma once

#include <QDialog>
#include <QJsonObject>
#include <map>

#include "chat/client/define.h"
#include "chat/client/util/input_check.h"

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
    // 显示提示
    void showTip(const QString& str, bool normal);
    void showErrTip(std::optional<std::string_view> str);

    // 初始化 http 回复处理
    void initHttpHandlers();

private:
    Ui::ResetDialog* ui;
    InputCheck input_check_;
    std::map<ReqId, std::function<void(const QJsonObject&)>> handlers_;
};