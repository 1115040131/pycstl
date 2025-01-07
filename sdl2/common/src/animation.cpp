#include "sdl2/common/animation.h"

#include <iostream>

#include <fmt/ostream.h>

namespace pyc {
namespace sdl2 {

Animation::Animation() {
    timer_.set_one_shot(false);
    timer_.set_on_timeout([&]() {
        frame_idx_++;
        if (frame_idx_ >= frames_.size()) {
            if (is_loop_) {
                frame_idx_ = 0;
            } else {
                frame_idx_ = frames_.size() - 1;
                if (on_finished_) {
                    on_finished_();
                }
            }
        }
    });
}

void Animation::add_frame(SDL_Texture* texture, int num) {
    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    int width_frame = width / num;

    for (int i = 0; i < num; i++) {
        frames_.emplace_back(SDL_Rect{i * width_frame, 0, width_frame, height}, texture);
    }
}

void Animation::add_frame(const Atlas& atlas) {
    for (size_t i = 0; i < atlas.size(); i++) {
        SDL_Texture* texture = atlas.get_texture(i);

        int width;
        int height;
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

        frames_.emplace_back(SDL_Rect{0, 0, width, height}, texture);
    }
}

void Animation::on_render(const Camera& camera) const {
    if (frames_.empty()) {
        fmt::println(std::cerr, "frames empty!");
        return;
    }

    const auto& frame = frames_[frame_idx_];

    SDL_FRect rect_dst{
        static_cast<float>(position_.x() - frame.rect_src.w / 2),
        static_cast<float>(position_.y() - frame.rect_src.h / 2),
        static_cast<float>(frame.rect_src.w),
        static_cast<float>(frame.rect_src.h),
    };

    camera.render_texture(frame.texture, &frame.rect_src, &rect_dst, angle_, &center_);
}

}  // namespace sdl2
}  // namespace pyc