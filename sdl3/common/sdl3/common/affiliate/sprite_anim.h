#include <chrono>

#include "sdl3/common/affiliate/sprite.h"

namespace pyc {
namespace sdl3 {

class SpriteAnim : public Sprite {
public:
    static SpriteAnim* Create(ObjectScreen* parent, const std::string& file_path, float scale = 1.F,
                              float fps = 10.F);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual void setTexture(const Texture& texture) override;

    int getFps() const { return fps_; }
    void setFps(int fps) { fps_ = fps; }

    void syncFrame(const SpriteAnim& other);

private:
    float fps_{10};

    int current_frame_{};
    int total_frames_{};
    std::chrono::duration<float> elapsed_time_{};
};

}  // namespace sdl3
}  // namespace pyc