#pragma once

#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class ObjectAffiliate : public Object {
public:
    enum class Anchor {
        kNone,
        kTopLeft,
        kTopCenter,
        kTopRight,
        kCenterLeft,
        kCenter,
        kCenterRight,
        kBottomLeft,
        kBottomCenter,
        kBottomRight,
    };

    const glm::vec2& getOffset() const { return offset_; }
    void setOffset(const glm::vec2& offset) {
        offset_ = offset;
        anchor_ = Anchor::kNone;
    }

    const glm::vec2& getSize() const { return size_; }
    void setSize(const glm::vec2& size) {
        size_ = size;
        setOffsetByAnchor(anchor_);
    }

    void setScale(float scale) { setSize(size_ * scale); }

    void setOffsetByAnchor(Anchor anchor);

    glm::vec2 getRenderPosition() const;

    glm::vec2 getPosition() const;

protected:
    glm::vec2 offset_{};
    glm::vec2 size_{};
    Anchor anchor_{Anchor::kCenter};
};

}  // namespace sdl3
}  // namespace pyc