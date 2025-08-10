#pragma once

#include <chrono>

#include "ghost_escape/affiliate/sprite.h"

namespace pyc {
namespace sdl3 {

class ObjectScreen;

class SpriteAnim : public Sprite {
public:
    static SpriteAnim* CreateAndSet(ObjectScreen* parent, const std::string& file_path, float scale = 1.F,
                                    float fps = 10.F, bool is_loop = true, Anchor anchor = Anchor::kCenter);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual void setTexture(const Texture& texture) override;

    int getFps() const { return fps_; }
    void setFps(int fps) { fps_ = fps; }

    bool isLoop() const { return is_loop_; }
    void setLoop(bool is_loop) { is_loop_ = is_loop; }

    void syncFrame(const SpriteAnim& other);

private:
    float fps_{10};
    bool is_loop_{true};

    int current_frame_{};
    int total_frames_{};
    std::chrono::duration<float> elapsed_time_{};
};

}  // namespace sdl3
}  // namespace pyc