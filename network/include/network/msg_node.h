#pragma once

#include <cstring>

namespace network {

class MsgNode {
    friend class Session;

public:
    MsgNode(char* msg, size_t max_len) : total_len_(max_len) {
        data_ = new char[max_len];
        memcpy(data_, msg, max_len);
    }

    ~MsgNode() { delete[] data_; }

private:
    size_t cur_len_ = 0;    // 已经接收/发送的数据长度
    size_t total_len_ = 0;  // 数据总长度
    char* data_ = nullptr;  // 数据缓冲区
};

}  // namespace network
