#pragma once

#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class BgStar : public Object {
public:
    static BgStar* CreateAndSet(Object* parent, int num, float far_scale, float mid_scale, float near_scale);

    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    struct StarData {
        std::vector<glm::vec2> stars;
        float scale = 1.0f;
        SDL_FColor color = {0, 0, 0, 1};
    };

    StarData far_stars_{};
    StarData mid_stars_{};
    StarData near_stars_{};

    std::chrono::duration<float> timer_{};
    int num_ = 2000;  // 每一层的星星数量
};

}  // namespace sdl3
}  // namespace pyc