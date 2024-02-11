#include "network/msg_node.h"

#include <boost/asio.hpp>

#include "common/utils.h"

namespace network {

namespace asio = boost::asio;

MsgHead MsgHead::ParseHead(const char* data) {
    using MsgType = std::pair<std::underlying_type<MsgId>::type, MsgSizeType>;
    const auto head = *reinterpret_cast<const MsgType*>(data);
    return MsgHead{static_cast<MsgId>(asio::detail::socket_ops::network_to_host_short(head.first)),
                   asio::detail::socket_ops::network_to_host_short(head.second)};
}

SendNode::SendNode(const char* msg, MsgSizeType max_len, MsgId msg_id) : MsgNode(max_len + kHeadLength) {
    using MsgType = std::pair<std::underlying_type<MsgId>::type, MsgSizeType>;
    *reinterpret_cast<MsgType*>(data_) =
        MsgType{asio::detail::socket_ops::host_to_network_short(pyc::ToUnderlying(msg_id)),
                asio::detail::socket_ops::host_to_network_short(max_len)};
    ::memcpy(data_ + kHeadLength, msg, max_len);
}

}  // namespace network
