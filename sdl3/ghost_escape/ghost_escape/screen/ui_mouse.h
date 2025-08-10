#pragma once

#include "ghost_escape/affiliate/sprite.h"
#include "ghost_escape/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class UIMouse : public ObjectScreen {
public:
    static UIMouse* CreateAndSet(Object* parent, const std::string& file_path1, const std::string& file_path2,
                                 float scale = 1.0f, Anchor anchor = Anchor::kCenter);

    virtual void update(std::chrono::duration<float> delta) override;

protected:
    Sprite* sprite1_{};
    Sprite* sprite2_{};
    std::chrono::duration<float> timer_{};
};

}  // namespace sdl3
}  // namespace pyc