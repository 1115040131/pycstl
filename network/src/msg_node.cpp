#include "network/msg_node.h"

#include <utility>

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;

namespace {

// 网络字节序的头部内存布局，纯 POD（trivially-copyable），可安全 memcpy 且字段按名访问
struct RawHead {
    MsgSizeType id;
    MsgSizeType length;
};
static_assert(sizeof(RawHead) == kHeadLength, "RawHead 必须与头部长度一致");

}  // namespace

MsgHead MsgHead::ParseHead(const char* data) {
    RawHead raw;
    ::memcpy(&raw, data, sizeof(raw));
    return MsgHead{static_cast<MsgId>(asio::detail::socket_ops::network_to_host_short(raw.id)),
                   asio::detail::socket_ops::network_to_host_short(raw.length)};
}

std::size_t MsgNode::Copy(const char* src, std::size_t len) {
    std::size_t copy_len = std::min(len, Remain());
    ::memcpy(data_.get() + cur_len_, src, copy_len);
    cur_len_ += copy_len;
    return copy_len;
}

SendNode::SendNode(const char* msg, MsgSizeType max_len, MsgId msg_id) : MsgNode(max_len + kHeadLength) {
    RawHead raw{asio::detail::socket_ops::host_to_network_short(std::to_underlying(msg_id)),
                asio::detail::socket_ops::host_to_network_short(max_len)};
    ::memcpy(data_.get(), &raw, sizeof(raw));
    ::memcpy(data_.get() + kHeadLength, msg, max_len);
}

}  // namespace network
