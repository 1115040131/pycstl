#include "sdl3/common/screen/hud_skill.h"

namespace pyc {
namespace sdl3 {

HUDSkill* HUDSkill::CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& render_position,
                                 float scale, Anchor anchor) {
    auto skill = std::make_unique<HUDSkill>();
    skill->init();
#ifdef DEBUG_MODE
    skill->SET_NAME(HUDSkill);
#endif
    skill->icon_ = Sprite::CreateAndSet(skill.get(), file_path, scale, anchor);
    skill->setRenderPosition(render_position);
    return static_cast<HUDSkill*>(parent->addChild(std::move(skill)));
}

void HUDSkill::render() {
    // 浅色背景
    SDL_SetTextureColorModFloat(icon_->getTexture().texture, 0.3, 0.3, 0.3);
    auto position = getRenderPosition() + icon_->getOffset();
    game_.renderTexture(icon_->getTexture(), position, icon_->getSize());
    SDL_SetTextureColorModFloat(icon_->getTexture().texture, 1.0, 1.0, 1.0);

    // 正常绘制
    ObjectScreen::render();
}

void HUDSkill::setPercent(float percent) {
    if (icon_) {
        icon_->setPercent(glm::vec2(1.0f, percent));
    }
}

}  // namespace sdl3
}  // namespace pyc