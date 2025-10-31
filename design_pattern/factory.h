#pragma once

#include <memory>

namespace pyc {

struct Bullet {
    virtual ~Bullet() = default;
    virtual void explode() = 0;
};

struct AK47Bullet : Bullet {
    void explode() override { std::puts("AK47 bullet explode"); }
};

struct MagicBullet : Bullet {
    void explode() override { std::puts("Magic bullet explode"); }
};

struct Gun {
    virtual ~Gun() = default;
    virtual std::unique_ptr<Bullet> shoot() = 0;
};

struct AK47 : Gun {
    std::unique_ptr<Bullet> shoot() override { return std::make_unique<AK47Bullet>(); }
};

struct MagicGun : Gun {
    std::unique_ptr<Bullet> shoot() override { return std::make_unique<MagicBullet>(); }
};

// 通过模版批量定义工厂
template <typename B>
struct GunWithBullet : Gun {
    static_assert(std::is_base_of<Bullet, B>::value, "B must be derived from Bullet");

    std::unique_ptr<Bullet> shoot() override { return std::make_unique<B>(); }
};

// 超级工厂
inline std::unique_ptr<Gun> getGun(std::string name) {
    if (name == "AK47") {
        return std::make_unique<AK47>();
    } else if (name == "MagicGun") {
        return std::make_unique<MagicGun>();
    } else {
        throw std::runtime_error("Unknown gun name");
    }
}

}  // namespace pyc
