module;

#include <iterator>
#include <map>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "ghost_escape/core/set_name.h"

module ghost_escape.affiliate.affiliate_bar;

import ghost_escape.core;

namespace pyc::sdl3 {

AffiliateBar* AffiliateBar::CreateAndSet(ObjectScreen* parent, const glm::vec2& size, Anchor anchor) {
    auto bar = std::make_unique<AffiliateBar>();
    bar->init();
#ifdef DEBUG_MODE
    bar->SET_NAME(AffiliateBar);
#endif
    bar->setSize(size);
    bar->setOffsetByAnchor(anchor);
    return static_cast<AffiliateBar*>(parent->addChild(std::move(bar)));
}

void AffiliateBar::render() {
    auto position = getRenderPosition();
    auto color = color_map_.lower_bound(percentage_);
    if (color == color_map_.end()) {
        color = std::prev(color);
    }
    game_.renderHBar(position, size_, percentage_, color->second);
}

}  // namespace pyc::sdl3
