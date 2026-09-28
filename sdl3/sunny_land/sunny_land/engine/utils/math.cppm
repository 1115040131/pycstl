module;

#include <glm/glm.hpp>

export module sunny_land.engine.utils.math;

export namespace pyc::sunny_land {

struct Rect {
    glm::vec2 position;
    glm::vec2 size;
};

struct FColor {
    float r;
    float g;
    float b;
    float a;
};

}  // namespace pyc::sunny_land
