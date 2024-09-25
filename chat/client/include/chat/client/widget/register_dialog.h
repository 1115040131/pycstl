#pragma once

#include <QDialog>
#include <QTimer>
#include <functional>
#include <memory>

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

signals:
    void switchLogin();

private slots:
    // 获取验证码按钮点击
    void on_get_code_btn_clicked();

    // 确认注册按钮点击
    void on_sure_btn_clicked();

    // 取消注册按钮点击
    void on_cancel_btn_clicked();

    // 返回登录按钮点击
    void on_return_btn_clicked();

    // http 请求完成
    void slot_reg_mod_finish(ReqId req_id, const QString& res, ErrorCode err);

private:
    // 初始化 http 回复处理
    void initHttpHandlers();

    // 切换页面
    void changeTipPage();

private:
    static constexpr Module kModule{Module::kRegisterMod};

    Ui::RegisterDialog* ui;
    std::map<ReqId, std::function<void(const QJsonObject&)>> handlers_;

    std::unique_ptr<QTimer> countdown_timer_;
    int countdown_{5};
};