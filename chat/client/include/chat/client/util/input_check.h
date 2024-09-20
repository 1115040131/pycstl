#pragma once

#include <QString>
#include <map>
#include <optional>
#include <string_view>

#include "chat/client/define.h"

class InputCheck {
public:
    std::optional<std::string_view> checkUserValid(const QString& user);

    std::optional<std::string_view> checkEmailValid(const QString& email);

    std::optional<std::string_view> checkPasswordValid(const QString& password);

    std::optional<std::string_view> checkConfirmValid(const QString& password, const QString& confirm);

    std::optional<std::string_view> checkVerifyValid(const QString& verify);

private:
    std::string_view addTipErr(TipErr tip_err, std::string_view msg);
    std::optional<std::string_view> delTipErr(TipErr tip_err);

private:
    static constexpr std::string_view kEmailRegex{R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"};
    static constexpr std::string_view kPasswordRegex{R"(^[a-zA-Z0-9!@#$%^&*]{6,15}$)"};

    std::map<TipErr, std::string_view> tip_errs_;
};