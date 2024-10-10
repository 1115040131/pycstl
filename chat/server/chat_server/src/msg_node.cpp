#include "chat/server/chat_server/msg_node.h"

#include "chat/server/chat_server/define.h"
#include "common/utils.h"

namespace pyc {
namespace chat {

MsgHead MsgHead::ParseHead(const char* data) {
    using MsgType = std::pair<std::underlying_type_t<ReqId>, uint16_t>;
    const auto head = *reinterpret_cast<const MsgType*>(data);
    return MsgHead{static_cast<ReqId>(boost::asio::detail::socket_ops::network_to_host_short(head.first)),
                   boost::asio::detail::socket_ops::network_to_host_short(head.second)};
}

std::size_t MsgNode::Copy(const char* src, std::size_t len) {
    std::size_t copy_len = std::min(len, Remain());
    ::memcpy(data_ + cur_len_, src, copy_len);
    cur_len_ += copy_len;
    return copy_len;
}

SendNode::SendNode(const char* msg, uint16_t max_len, ReqId msg_id) : MsgNode(max_len + kHeadLength) {
    using MsgType = std::pair<std::underlying_type<ReqId>::type, uint16_t>;
    *reinterpret_cast<MsgType*>(data_) =
        MsgType{boost::asio::detail::socket_ops::host_to_network_short(ToUnderlying(msg_id)),
                boost::asio::detail::socket_ops::host_to_network_short(max_len)};
    ::memcpy(data_ + kHeadLength, msg, max_len);
}

}  // namespace chat
}  // namespace pyc
