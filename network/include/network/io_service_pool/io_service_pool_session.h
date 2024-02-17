#pragma once

#include "network/base/session.h"

namespace network {

class IOServicePoolSession : public Session {
public:
    using Session::Session;

    ~IOServicePoolSession() override;

    void Start() override;

protected:
    void AsyncRead() override;
    void AsyncWrite() override;
};

}  // namespace network
