#include "chat/client/util/err_tip.h"

#include <QRegularExpression>

#include "chat/client/api.h"

ErrTip::ErrTip(QWidget* parent) : QLabel(parent) {
    setProperty("state", "normal");
    repolish(this);
}

void ErrTip::showTip(const QString& str, bool normal) {
    setText(str);
    if (normal) {
        setProperty("state", "normal");
    } else {
        setProperty("state", "err");
    }
    repolish(this);
}

void ErrTip::addTipErr(TipErr tip_err, const QString& msg) {
    tip_errs_[tip_err] = msg;
    showTip(msg, false);
}

bool ErrTip::delTipErr(TipErr tip_err) {
    tip_errs_.erase(tip_err);
    if (tip_errs_.empty()) {
        clear();
        return true;
    }
    showTip(tip_errs_.begin()->second, false);
    return false;
}

bool ErrTip::checkUserValid(const QString& user) {
    if (user == "") {
        addTipErr(TipErr::kUserErr, tr("用户名不能为空"));
        return false;
    }

    return delTipErr(TipErr::kUserErr);
}

bool ErrTip::checkEmailValid(const QString& email) {
    QRegularExpression regex(kEmailRegex.data());
    bool match = regex.match(email).hasMatch();
    if (!match) {
        addTipErr(TipErr::kEmailErr, "邮箱地址不正确");
        return false;
    }

    return delTipErr(TipErr::kEmailErr);
}

bool ErrTip::checkPasswordValid(const QString& password) {
    if (password.length() < 6 || password.length() > 15) {
        addTipErr(TipErr::kPasswordErr, "密码长度应为6-15");
        return false;
    }

    QRegularExpression regex(kPasswordRegex.data());
    bool match = regex.match(password).hasMatch();
    if (!match) {
        addTipErr(TipErr::kPasswordErr, "不能包含非法字符");
        return false;
    }

    return delTipErr(TipErr::kPasswordErr);
}

bool ErrTip::checkConfirmValid(const QString& password, const QString& confirm) {
    if (password != confirm) {
        addTipErr(TipErr::kPasswordConfirm, "密码和确认密码不匹配");
        return false;
    }

    return delTipErr(TipErr::kPasswordConfirm);
}

bool ErrTip::checkVerifyValid(const QString& verify) {
    if (verify == "") {
        addTipErr(TipErr::kVerifyErr, "验证码不能为空");
        return false;
    }

    return delTipErr(TipErr::kVerifyErr);
}
