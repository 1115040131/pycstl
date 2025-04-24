#include "sdl3/common/screen/ui_mouse.h"

namespace pyc {
namespace sdl3 {

UIMouse* UIMouse::CreateAndSet(Object* parent, const std::string& file_path1, const std::string& file_path2,
                               float scale, Anchor anchor) {
    auto ui_mouse = std::make_unique<UIMouse>();
    ui_mouse->init();
#ifdef DEBUG_MODE
    ui_mouse->SET_NAME(UIMouse);
#endif
    ui_mouse->sprite1_ = Sprite::CreateAndSet(ui_mouse.get(), file_path1, scale, anchor);
    ui_mouse->sprite2_ = Sprite::CreateAndSet(ui_mouse.get(), file_path2, scale, anchor);
    return static_cast<UIMouse*>(parent->addChild(std::move(ui_mouse)));
}

void UIMouse::update(std::chrono::duration<float> delta) {
    timer_ += delta;
    if (timer_ < 0.3s) {
        sprite1_->setActive(true);
        sprite2_->setActive(false);
    } else if (timer_ < 0.6s) {
        sprite1_->setActive(false);
        sprite2_->setActive(true);
    } else {
        timer_ = 0s;
    }
    setRenderPosition(game_.getMousePosition());
}

}  // namespace sdl3
}  // namespace pyc