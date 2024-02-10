#include "network/msg_node.h"

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;

MsgHead MsgHead::MakeNetworkData(MsgSizeType id, MsgSizeType length) {
    return MsgHead{asio::detail::socket_ops::host_to_network_short(id),
                   asio::detail::socket_ops::host_to_network_short(length)};
}

MsgHead MsgHead::ParseHead(const char* data) {
    const auto head = *reinterpret_cast<const MsgHead*>(data);
    return MsgHead{asio::detail::socket_ops::network_to_host_short(head.id),
                   asio::detail::socket_ops::network_to_host_short(head.length)};
}

RecvNode::RecvNode(MsgSizeType max_len) : MsgNode(max_len) {}

SendNode::SendNode(const char* msg, MsgSizeType max_len, MsgSizeType msg_id) : MsgNode(max_len + kHeadLength) {
    *reinterpret_cast<MsgHead*>(data_) = MsgHead::MakeNetworkData(msg_id, max_len);
    ::memcpy(data_ + kHeadLength, msg, max_len);
}

}  // namespace network
