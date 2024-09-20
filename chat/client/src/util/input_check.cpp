#include "chat/client/util/input_check.h"

#include <QRegularExpression>

std::string_view InputCheck::addTipErr(TipErr tip_err, std::string_view msg) {
    tip_errs_[tip_err] = msg;
    return msg;
}

std::optional<std::string_view> InputCheck::delTipErr(TipErr tip_err) {
    tip_errs_.erase(tip_err);
    if (tip_errs_.empty()) {
        return std::nullopt;
    }
    return tip_errs_.begin()->second;
}

std::optional<std::string_view> InputCheck::checkUserValid(const QString& user) {
    if (user == "") {
        return addTipErr(TipErr::kUserErr, "用户名不能为空");
    }

    return delTipErr(TipErr::kUserErr);
}

std::optional<std::string_view> InputCheck::checkEmailValid(const QString& email) {
    QRegularExpression regex(kEmailRegex.data());
    bool match = regex.match(email).hasMatch();
    if (!match) {
        return addTipErr(TipErr::kEmailErr, "邮箱地址不正确");
    }

    return delTipErr(TipErr::kEmailErr);
}

std::optional<std::string_view> InputCheck::checkPasswordValid(const QString& password) {
    if (password.length() < 6 || password.length() > 15) {
        return addTipErr(TipErr::kPasswordErr, "密码长度应为6-15");
    }

    QRegularExpression regex(kPasswordRegex.data());
    bool match = regex.match(password).hasMatch();
    if (!match) {
        return addTipErr(TipErr::kPasswordErr, "不能包含非法字符");
    }

    return delTipErr(TipErr::kPasswordErr);
}

std::optional<std::string_view> InputCheck::checkConfirmValid(const QString& password, const QString& confirm) {
    if (password != confirm) {
        return addTipErr(TipErr::kPasswordConfirm, "密码和确认密码不匹配");
    }

    return delTipErr(TipErr::kPasswordConfirm);
}

std::optional<std::string_view> InputCheck::checkVerifyValid(const QString& verify) {
    if (verify == "") {
        return addTipErr(TipErr::kVerifyErr, "验证码不能为空");
    }

    return delTipErr(TipErr::kVerifyErr);
}