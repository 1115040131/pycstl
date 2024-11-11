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

static const std::vector<QString> strs = {"hello world !", "nice to meet u", "New year, new life",
                                          "You have to love yourself",
                                          "My love is written in the wind ever since the whole world is you"};
static const std::vector<QString> heads = {"chat/client/res/head_1.jpg", "chat/client/res/head_2.jpg",
                                           "chat/client/res/head_3.jpg", "chat/client/res/head_4.jpg",
                                           "chat/client/res/head_5.jpg"};
static const std::vector<QString> names = {"llfc", "zack", "golang", "cpp", "java", "nodejs", "python", "rust"};