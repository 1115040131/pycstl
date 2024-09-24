#pragma once

#include <QLabel>
#include <map>

#include "chat/client/define.h"

class ErrTip : public QLabel {
    Q_OBJECT

public:
    explicit ErrTip(QWidget* parent = nullptr);

    void showTip(const QString& str, bool normal);

    bool checkUserValid(const QString& user);

    bool checkEmailValid(const QString& email);

    bool checkPasswordValid(const QString& password);

    bool checkConfirmValid(const QString& password, const QString& confirm);

    bool checkVerifyValid(const QString& verify);

private:
    void addTipErr(TipErr tip_err, const QString& msg);

    bool delTipErr(TipErr tip_err);

private:
    static constexpr std::string_view kEmailRegex{R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"};
    static constexpr std::string_view kPasswordRegex{R"(^[a-zA-Z0-9!@#$%^&*]{6,15}$)"};

    std::map<TipErr, QString> tip_errs_;
};