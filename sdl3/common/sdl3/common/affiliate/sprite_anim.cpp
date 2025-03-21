#include "sdl3/common/affiliate/sprite_anim.h"

namespace pyc {
namespace sdl3 {

using namespace std::chrono_literals;

std::shared_ptr<SpriteAnim> SpriteAnim::Create(ObjectScreen* parent, const std::string& file_path, float scale,
                                               float fps) {
    auto sprite = std::make_shared<SpriteAnim>();
    sprite->init();
    sprite->setParent(parent);
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    sprite->setFps(fps);
    parent->addChild(sprite);
    return sprite;
}

void SpriteAnim::update(std::chrono::duration<float> delta) {
    elapsed_time_ += delta;
    while (elapsed_time_ > 1.0s / fps_) {
        elapsed_time_ -= 1.0s / fps_;
        current_frame_ = (current_frame_ + 1) % total_frames_;
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
