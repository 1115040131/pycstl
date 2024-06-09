#pragma once

#include <memory>

namespace pyc {

struct Ball {
    virtual std::unique_ptr<Ball> clone() = 0;
};

template <typename Derived>
struct BallImpl : Ball {  // 自动实现 clone 的辅助工具类
    std::unique_ptr<Ball> clone() override {
        return std::make_unique<Derived>(static_cast<Derived const&>(*this));
    }
};

struct RedBall : BallImpl<RedBall> {
    // std::unique_ptr<Ball> clone() override {
    //     return std::make_unique<RedBall>(*this);
    // }
};

struct BlueBall : BallImpl<BlueBall> {
    // std::unique_ptr<Ball> clone() override {
    //     return std::make_unique<BlueBall>(*this);
    // }
};

}  // namespace pyc
