#pragma once

#include <QPixmap>
#include <QString>

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

enum class ChatRole {
    kSelf,
    kOther,
};

struct MsgInfo {
    enum class Type {
        kText,
        kImage,
        kFile,
    };
    Type type;
    QString content;
    QPixmap pixmap;
};

struct ServerInfo {
    int uid;
    QString host;
    QString port;
    QString token;
};