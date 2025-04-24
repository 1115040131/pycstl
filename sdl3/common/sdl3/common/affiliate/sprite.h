#pragma once

#include "sdl3/common/core/object_affiliate.h"
#include "sdl3/common/core/texture.h"

namespace pyc {
namespace sdl3 {

class ObjectScreen;

class Sprite : public ObjectAffiliate {
public:
    static Sprite* CreateAndSet(ObjectScreen* parent, const std::string& file_path, float scale = 1.F,
                                Anchor anchor = Anchor::kCenter);

    virtual void render() override;

    bool isFinish() const { return is_finish_; }
    void setFinish(bool is_finish) { is_finish_ = is_finish; }

    void setPercent(const glm::vec2& percent) { percent_ = percent; }

    const Texture& getTexture() const { return texture_; }
    virtual void setTexture(const Texture& texture);

    void setAngle(float angle) { texture_.angle = angle; }
    void setFlip(bool flip) { texture_.is_flip = flip; }

protected:
    bool is_finish_{};
    glm::vec2 percent_{1, 1};

    Texture texture_;
};

}  // namespace sdl3
}  // namespace pyc
