#include "sdl3/common/raw/bg_star.h"

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

BgStar* BgStar::CreateAndSet(Object* parent, int num, float far_scale, float mid_scale, float near_scale) {
    auto bg_star = std::make_unique<BgStar>();
    bg_star->init();
#ifdef DEBUG_MODE
    bg_star->SET_NAME(BgStar);
#endif
    bg_star->num_ = num;
    bg_star->far_stars_.scale = far_scale;
    bg_star->mid_stars_.scale = mid_scale;
    bg_star->near_stars_.scale = near_scale;

    bg_star->far_stars_.stars.reserve(num);
    bg_star->mid_stars_.stars.reserve(num);
    bg_star->near_stars_.stars.reserve(num);
    auto& game = Game::GetInstance();
    auto extra = game.getCurrentScene()->getWorldSize() - game.getScreenSize();
    for (int i = 0; i < num; i++) {
        bg_star->far_stars_.stars.emplace_back(game.random(glm::vec2(), game.getScreenSize() + extra * far_scale));
        bg_star->mid_stars_.stars.emplace_back(game.random(glm::vec2(), game.getScreenSize() + extra * mid_scale));
        bg_star->near_stars_.stars.emplace_back(
            game.random(glm::vec2(), game.getScreenSize() + extra * near_scale));
    }

    return static_cast<BgStar*>(parent->addChild(std::move(bg_star)));
}

void BgStar::update(std::chrono::duration<float> delta) {
    timer_ += delta;
    far_stars_.color = {0.5f + 0.5f * std::sin(timer_.count() * 0.9f),
                        0.5f + 0.5f * std::sin(timer_.count() * 0.8f),
                        0.5f + 0.5f * std::sin(timer_.count() * 0.7f), 1.0f};
    mid_stars_.color = {0.5f + 0.5f * std::sin(timer_.count() * 0.8f),
                        0.5f + 0.5f * std::sin(timer_.count() * 0.7f),
                        0.5f + 0.5f * std::sin(timer_.count() * 0.6f), 1.0f};
    near_stars_.color = {0.5f + 0.5f * std::sin(timer_.count() * 0.7f),
                         0.5f + 0.5f * std::sin(timer_.count() * 0.6f),
                         0.5f + 0.5f * std::sin(timer_.count() * 0.5f), 1.0f};
}

void BgStar::render() {
    const auto& camera_position = game_.getCurrentScene()->getCameraPosition();
    game_.drawPoints(far_stars_.stars, -camera_position * far_stars_.scale, far_stars_.color);
    game_.drawPoints(mid_stars_.stars, -camera_position * mid_stars_.scale, mid_stars_.color);
    game_.drawPoints(near_stars_.stars, -camera_position * near_stars_.scale, near_stars_.color);
}

}  // namespace sdl3
}  // namespace pyc