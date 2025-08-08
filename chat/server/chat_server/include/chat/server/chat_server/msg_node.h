#pragma once

#include <cstring>

#include "chat/common/method.h"
#include "common/noncopyable.h"

namespace pyc {
namespace chat {

struct MsgHead {
    ReqId id;
    uint16_t length;

    /// @brief 从网络数据中解析出 id 和 length
    static MsgHead ParseHead(const char* data);
};

inline constexpr size_t kHeadLength = sizeof(MsgHead);  // tlv 头部长度
inline constexpr size_t kMaxLength = 2ul * 1024;        // 消息体最大长度 2KB

class MsgNode : public Noncopyable {
public:
    MsgNode(uint16_t max_len) : total_len_(max_len) {
        data_ = new char[total_len_ + 1];
        data_[total_len_] = '\0';
    }

    ~MsgNode() { delete[] data_; }

    size_t Remain() const { return total_len_ - cur_len_; }

    size_t Size() const { return total_len_; }

    char* Data() { return data_; }

    const char* Data() const { return data_; }

    size_t Copy(const char* src, size_t len);

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
public:
    RecvNode(uint16_t max_len, ReqId msg_id) : MsgNode(max_len), msg_id_(msg_id) {}

    ReqId GetReqId() const { return msg_id_; }

private:
    ReqId msg_id_;
};

class SendNode : public MsgNode {
public:
    SendNode(const char* msg, uint16_t max_len, ReqId msg_id);
};

}  // namespace chat
}  // namespace pyc
