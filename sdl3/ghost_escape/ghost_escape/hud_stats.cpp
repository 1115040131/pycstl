#include "ghost_escape/hud_stats.h"

namespace pyc {
namespace sdl3 {

HUDStatus* HUDStatus::CreateAndSet(Object* parent, Actor* target, glm::vec2 render_position) {
    auto hud_stats = std::make_unique<HUDStatus>();
    hud_stats->init();
#ifdef DEBUG_MODE
    hud_stats->SET_NAME(HUDStatus);
#endif
    hud_stats->target_ = target;
    hud_stats->setRenderPosition(render_position);
    return static_cast<HUDStatus*>(parent->addChild(std::move(hud_stats)));
}

void HUDStatus::init() {
    ObjectScreen::init();
    health_bar_bg_ = Sprite::CreateAndSet(this, ASSET("UI/bar_bg.png"), 3.0f, Anchor::kCenterLeft);
    health_bar_bg_->addOffset(glm::vec2(30, 0));
    health_bar_ = Sprite::CreateAndSet(this, ASSET("UI/bar_red.png"), 3.0f, Anchor::kCenterLeft);
    health_bar_->addOffset(glm::vec2(30, 0));
    health_icon_ = Sprite::CreateAndSet(this, ASSET("UI/Red Potion.png"), 0.5f, Anchor::kCenterLeft);

    mana_bar_bg_ = Sprite::CreateAndSet(this, ASSET("UI/bar_bg.png"), 3.0f, Anchor::kCenterLeft);
    mana_bar_bg_->addOffset(glm::vec2(300, 0));
    mana_bar_ = Sprite::CreateAndSet(this, ASSET("UI/bar_blue.png"), 3.0f, Anchor::kCenterLeft);
    mana_bar_->addOffset(glm::vec2(300, 0));
    mana_icon_ = Sprite::CreateAndSet(this, ASSET("UI/Blue Potion.png"), 0.5f, Anchor::kCenterLeft);
    mana_icon_->addOffset(glm::vec2(270, 0));
}

void HUDStatus::update(std::chrono::duration<float> delta) {
    ObjectScreen::update(delta);
    if (target_ && target_->getStats()) {
        health_bar_->setPercent(
            glm::vec2(target_->getStats()->getHealth() / target_->getStats()->getMaxHealth(), 1.0f));
        mana_bar_->setPercent(glm::vec2(target_->getStats()->getMana() / target_->getStats()->getMaxMana(), 1.0f));
    }
}

}  // namespace sdl3
}  // namespace pyc