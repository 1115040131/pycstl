#pragma once

#include <fmt/format.h>

#include "network/msg_node.h"
#include "network/proto/msg.pb.h"

namespace network {

const char* ToString(MsgId msg_id);

}  // namespace network

// fmt 格式化
namespace fmt {

template <>
struct formatter<network::MsgId> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::MsgId& msg_id, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", network::ToString(msg_id));
    }
};

template <>
struct formatter<network::MsgHead> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::MsgHead& head, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "MsgHead(id: {}, length: {})", network::ToString(head.id), head.length);
    }
};

template <>
struct formatter<network::MsgData> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::MsgData& msg_data, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "id = {}, data = \'{}\'", msg_data.id(), msg_data.data());
    }
};

}  // namespace fmt