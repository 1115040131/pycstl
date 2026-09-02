#include "shooter/scene.h"

#include "shooter/game.h"

namespace pyc {
namespace sdl2 {

Scene::Scene() : game_(Game::GetInstance()) {}

}  // namespace sdl2
}  // namespace pyc