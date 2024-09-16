#pragma once

#include <QDialog>
#include <QJsonObject>
#include <functional>
#include <map>

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

    // 输入框错误提示
    void addTipErr(TipErr tip_err, QString msg);
    void delTipErr(TipErr tip_err);
    void checkUserValid();
    void checkEmailValid();
    void checkPasswordValid();
    void checkConfirmValid();
    void checkVerifyValid();

private:
    static constexpr std::string_view kEmailRegex{R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"};
    static constexpr std::string_view kPasswordRegex{R"(^[a-zA-Z0-9!@#$%^&*]{6,15}$)"};

    Ui::RegisterDialog* ui;
    std::map<ReqId, std::function<void(const QJsonObject&)>> handlers_;
    std::map<TipErr, QString> tip_errs_;
};