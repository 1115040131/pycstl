#pragma once

#include <fmt/core.h>

#include "network/msg_node.h"
#include "network/proto/msg.pb.h"

template <>
struct fmt::formatter<network::MsgHead> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::MsgHead& head, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "id = {}, length = {}", head.id, head.length);
    }
};

template <>
struct fmt::formatter<network::MsgData> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const network::MsgData& msg_data, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "id = {}, data = \"{}\"", msg_data.id(), msg_data.data());
    }
};
