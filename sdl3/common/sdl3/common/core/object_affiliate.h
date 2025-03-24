#pragma once

#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class ObjectAffiliate : public Object {
public:
    const glm::vec2& getOffset() const { return offset_; }
    void setOffset(const glm::vec2& offset) { offset_ = offset; }

    const glm::vec2& getSize() const { return size_; }
    void setSize(const glm::vec2& size) { size_ = size; }

    void setScale(float scale) { size_ = size_ * scale; }

    glm::vec2 getRenderPosition() const;

    glm::vec2 getPosition() const;

protected:
    glm::vec2 offset_{};
    glm::vec2 size_{};
};

}  // namespace sdl3
}  // namespace pyc