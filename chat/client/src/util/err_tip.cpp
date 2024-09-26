#include "chat/client/util/err_tip.h"

#include <QJsonDocument>
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

void ErrTip::delTipErr(TipErr tip_err) {
    tip_errs_.erase(tip_err);
    if (tip_errs_.empty()) {
        clear();
    } else {
        showTip(tip_errs_.begin()->second, false);
    }
}

bool ErrTip::checkUserValid(const QString& user) {
    if (user == "") {
        addTipErr(TipErr::kUserErr, tr("用户名不能为空"));
        return false;
    }

    delTipErr(TipErr::kUserErr);
    return true;
}

bool ErrTip::checkEmailValid(const QString& email) {
    QRegularExpression regex(kEmailRegex.data());
    bool match = regex.match(email).hasMatch();
    if (!match) {
        addTipErr(TipErr::kEmailErr, "邮箱地址不正确");
        return false;
    }

    delTipErr(TipErr::kEmailErr);
    return true;
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

    delTipErr(TipErr::kPasswordErr);
    return true;
}

bool ErrTip::checkConfirmValid(const QString& password, const QString& confirm) {
    if (password != confirm) {
        addTipErr(TipErr::kPasswordConfirm, "密码和确认密码不匹配");
        return false;
    }

    delTipErr(TipErr::kPasswordConfirm);
    return true;
}

bool ErrTip::checkVerifyValid(const QString& verify) {
    if (verify == "") {
        addTipErr(TipErr::kVerifyErr, "验证码不能为空");
        return false;
    }

    delTipErr(TipErr::kVerifyErr);
    return true;
}

std::optional<QJsonDocument> ErrTip::checkHttpResponse(const QString& res, ErrorCode err) {
    if (err != ErrorCode::kSuccess) {
        showTip(tr("网络请求错误"), false);
        return std::nullopt;
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(res.toUtf8());
    if (json_doc.isEmpty()) {
        showTip(tr("json解析失败"), false);
        return std::nullopt;
    }
    if (!json_doc.isObject()) {
        showTip(tr("json解析失败"), false);
        return std::nullopt;
    }

    return json_doc;
}