#include "chat/server/chat_server/msg_node.h"

#include <utility>

#include <boost/asio.hpp>

namespace pyc {
namespace chat {

namespace {

// 网络字节序的头部内存布局，纯 POD（trivially-copyable），可安全 memcpy 且字段按名访问
struct RawHead {
    uint16_t id;
    uint16_t length;
};
static_assert(sizeof(RawHead) == kHeadLength, "RawHead 必须与头部长度一致");

}  // namespace

MsgHead MsgHead::ParseHead(const char* data) {
    RawHead raw;
    ::memcpy(&raw, data, sizeof(raw));
    return MsgHead{static_cast<ReqId>(boost::asio::detail::socket_ops::network_to_host_short(raw.id)),
                   boost::asio::detail::socket_ops::network_to_host_short(raw.length)};
}

std::size_t MsgNode::Copy(const char* src, std::size_t len) {
    std::size_t copy_len = std::min(len, Remain());
    ::memcpy(data_.get() + cur_len_, src, copy_len);
    cur_len_ += copy_len;
    return copy_len;
}

SendNode::SendNode(const char* msg, uint16_t max_len, ReqId msg_id) : MsgNode(max_len + kHeadLength) {
    RawHead raw{boost::asio::detail::socket_ops::host_to_network_short(std::to_underlying(msg_id)),
                boost::asio::detail::socket_ops::host_to_network_short(max_len)};
    ::memcpy(data_.get(), &raw, sizeof(raw));
    ::memcpy(data_.get() + kHeadLength, msg, max_len);
}

}  // namespace chat
}  // namespace pyc
