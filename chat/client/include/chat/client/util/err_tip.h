#pragma once

#include <QJsonDocument>
#include <QLabel>
#include <map>
#include <optional>

#include "chat/client/define.h"

class ErrTip : public QLabel {
    Q_OBJECT

public:
    explicit ErrTip(QWidget* parent = nullptr);

    void showTip(const QString& str, bool normal);

    // 校验用户名是否有效
    bool checkUserValid(const QString& user);

    // 校验邮箱是否有效
    bool checkEmailValid(const QString& email);

    // 校验密码是否有效
    bool checkPasswordValid(const QString& password);

    // 校验确认密码是否有效
    bool checkConfirmValid(const QString& password, const QString& confirm);

    // 校验验证码是否有效
    bool checkVerifyValid(const QString& verify);

    std::optional<QJsonDocument> checkHttpResponse(const QString& res, ErrorCode err);

private:
    void addTipErr(TipErr tip_err, const QString& msg);

    bool delTipErr(TipErr tip_err);

private:
    static constexpr std::string_view kEmailRegex{R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"};
    static constexpr std::string_view kPasswordRegex{R"(^[a-zA-Z0-9!@#$%^&*]{6,15}$)"};

    std::map<TipErr, QString> tip_errs_;
};