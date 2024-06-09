#include <memory>

#include <gtest/gtest.h>

#include "design_pattern/factory.h"

namespace pyc {

inline void player(std::unique_ptr<Gun> gun) {
    for (int i = 0; i < 3; i++) {
        auto bullet = gun->shoot();
        bullet->explode();
    }
}

TEST(DesignPatternTest, FactoryPattern) {
    player(std::make_unique<AK47>());
    player(std::make_unique<MagicGun>());
}

TEST(DesignPatternTest, TemplateFactory) {
    player(std::make_unique<GunWithBullet<AK47Bullet>>());
    player(std::make_unique<GunWithBullet<MagicBullet>>());
}

TEST(DesignPatternTest, SupperFactory) {
    player(getGun("AK47"));
    player(getGun("MagicGun"));
}

}  // namespace pyc