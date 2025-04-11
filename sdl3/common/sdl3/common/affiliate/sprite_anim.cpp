#include "sdl3/common/affiliate/sprite_anim.h"

#include "sdl3/common/core/object_screen.h"

namespace pyc {
namespace sdl3 {

SpriteAnim* SpriteAnim::CreateAndSet(ObjectScreen* parent, const std::string& file_path, float scale, float fps,
                                     bool is_loop, Anchor anchor) {
    auto sprite = std::make_unique<SpriteAnim>();
    sprite->init();
#ifdef DEBUG_MODE
    sprite->SET_NAME(SpriteAnim);
#endif
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    sprite->setFps(fps);
    sprite->setLoop(is_loop);
    sprite->setOffsetByAnchor(anchor);
    return static_cast<SpriteAnim*>(parent->addChild(std::move(sprite)));
}

void SpriteAnim::update(std::chrono::duration<float> delta) {
    if (is_finish_) {
        return;
    }
    elapsed_time_ += delta;
    while (elapsed_time_ > 1.0s / fps_) {
        elapsed_time_ -= 1.0s / fps_;
        current_frame_++;
        if (current_frame_ >= total_frames_) {
            if (is_loop_) {
                current_frame_ = 0;
            } else {
                current_frame_ = total_frames_ - 1;
                is_finish_ = true;
                break;
            }
        }
    }
    texture_.src_rect.x = texture_.src_rect.w * current_frame_;
}

void SpriteAnim::setTexture(const Texture& texture) {
    texture_ = texture;
    total_frames_ = texture.src_rect.w / texture.src_rect.h;
    texture_.src_rect.w = texture.src_rect.h;
    size_ = glm::vec2(texture_.src_rect.w, texture_.src_rect.h);
}

void SpriteAnim::syncFrame(const SpriteAnim& other) {
    current_frame_ = other.current_frame_;
    elapsed_time_ = other.elapsed_time_;
    texture_.src_rect.x = other.texture_.src_rect.x;
}

}  // namespace sdl3
}  // namespace pyc
