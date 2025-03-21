#pragma once

#include "sdl3/common/core/object.h"
#include "sdl3/common/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class ObjectAffiliate : public Object {
public:
    ObjectScreen* getParent() const { return parent_; }
    void setParent(ObjectScreen* parent) { parent_ = parent; }

    const glm::vec2& getOffset() const { return offset_; }
    void setOffset(const glm::vec2& offset) { offset_ = offset; }

    const glm::vec2& getSize() const { return size_; }
    void setSize(const glm::vec2& size) { size_ = size; }

protected:
    ObjectScreen* parent_{};
    glm::vec2 offset_{};
    glm::vec2 size_{};
};

}  // namespace sdl3
}  // namespace pyc