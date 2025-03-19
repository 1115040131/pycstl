#include "sdl3/common/scene.h"

namespace pyc {
namespace sdl3 {

void Scene::setCameraPosition(const glm::vec2& camera_position) {
    constexpr auto kOffset = glm::vec2(30.F);
    camera_position_ = glm::clamp(camera_position, -kOffset, world_size_ - game_.getScreenSize() + kOffset);
}

}  // namespace sdl3
}  // namespace pyc
