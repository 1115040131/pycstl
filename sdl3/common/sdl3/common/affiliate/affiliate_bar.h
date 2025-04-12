#pragma once

#include <map>

#include "sdl3/common/core/object_affiliate.h"

namespace pyc {
namespace sdl3 {

class AffiliateBar : public ObjectAffiliate {
public:
    static AffiliateBar* CreateAndSet(ObjectScreen* parent, const glm::vec2& size,
                                      Anchor anchor = Anchor::kCenter);

    virtual void render() override;

    void setPercentage(float percentage) { percentage_ = std::clamp(percentage, 0.0f, 1.0f); }

protected:
    float percentage_{1.0f};
    std::map<float, SDL_FColor> color_map_{
        {0.3f, {1, 0, 0, 1}},     // 红色
        {0.7f, {1, 0.65, 0, 1}},  // 橙色
        {1.0f, {0, 1, 0, 1}},     // 绿色
    };
};

}  // namespace sdl3
}  // namespace pyc