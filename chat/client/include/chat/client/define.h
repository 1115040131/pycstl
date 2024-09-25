#pragma once

#include "chat/common/error_code.h"
#include "chat/common/method.h"

enum class Module {
    kRegisterMod = 0,
    kResetMod,
    kLogin,
};

enum class TipErr {
    kSuccess = 0,
    kEmailErr,
    kPasswordErr,
    kConfirmErr,
    kPasswordConfirm,
    kVerifyErr,
    kUserErr,
};
