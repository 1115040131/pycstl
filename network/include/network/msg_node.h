#pragma once

#include <cstring>
#include <numeric>

namespace network {

using MsgSizeType = unsigned short;
static constexpr size_t kHeadLength = sizeof(MsgSizeType);
static constexpr size_t kMaxLength = std::min(static_cast<size_t>(std::numeric_limits<MsgSizeType>::max()), 2ul * 1024);

class MsgNode {
    friend class Session;

public:
    MsgNode(char* msg, MsgSizeType max_len) : total_len_(max_len + kHeadLength) {
        data_ = new char[total_len_ + 1];
        memcpy(data_, &max_len, kHeadLength);
        memcpy(data_ + kHeadLength, msg, max_len);
        data_[total_len_] = '\0';
    }

    MsgNode(MsgSizeType max_len) : total_len_(max_len) { data_ = new char[total_len_ + 1]; }

    ~MsgNode() { delete[] data_; }

    void Clear() {
        memset(data_, 0, total_len_);
        cur_len_ = 0;
    }

private:
    size_t cur_len_ = 0;    // 已经接收/发送的数据长度
    size_t total_len_ = 0;  // 数据总长度
    char* data_ = nullptr;  // 数据缓冲区
};

}  // namespace network
