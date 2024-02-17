#include "network/utils.h"

namespace network {

const char* ToString(MsgId msg_id) {
    switch (msg_id) {
        case MsgId::kMsgHelloWorld:
            return "MsgHelloWorld";
        default:
            break;
    }
    return "MsgUnknown";
}

}  // namespace network
