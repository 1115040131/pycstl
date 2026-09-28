module;

#include <chrono>
#include <string>

#include <glm/glm.hpp>

export module ghost_escape.screen.ui_mouse;

export import ghost_escape.core;
export import ghost_escape.affiliate.sprite;

export namespace pyc::sdl3 {

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

}  // namespace pyc::sdl3
