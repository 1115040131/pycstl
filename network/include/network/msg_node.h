#pragma once

#include <cstring>
#include <numeric>
#include <string>

#include "common/utils.h"

namespace network {

using MsgSizeType = unsigned short;

enum class MsgId : MsgSizeType {
    kMsgHelloWorld = 1001,

    kMaxId,
};

struct MsgHead {
    MsgId id;
    MsgSizeType length;

    /// @brief 从网络数据中解析出 id 和 length
    static MsgHead ParseHead(const char* data);
};

static constexpr size_t kHeadLength = sizeof(MsgHead);  // tlv 头部长度
static constexpr size_t kMaxLength = 2ul * 1024;        // 消息体最大长度 2KB
static constexpr size_t kMaxSendQueue = 1000;           // 发送队列最大长度

class MsgNode {
public:
    MsgNode(MsgSizeType max_len) : total_len_(max_len) {
        data_ = new char[total_len_ + 1];
        data_[total_len_] = '\0';
    }

    ~MsgNode() { delete[] data_; }

    size_t Size() const { return total_len_; }
    const char* Data() const { return data_; }

    void Clear() {
        ::memset(data_, 0, total_len_);
        cur_len_ = 0;
    }

protected:
    size_t cur_len_ = 0;    // 已经接收/发送的数据长度
    size_t total_len_ = 0;  // 数据总长度
    char* data_ = nullptr;  // 数据缓冲区
};

class RecvNode : public MsgNode {
    friend class Session;
    friend class LogicSystem;

public:
    RecvNode(MsgSizeType max_len, MsgId msg_id) : MsgNode(max_len), msg_id_(msg_id) {}

private:
    MsgId msg_id_;
};

class SendNode : public MsgNode {
    friend class Session;

public:
    SendNode(const char* msg, MsgSizeType max_len, MsgId msg_id);
};

}  // namespace network
