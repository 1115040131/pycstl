module;

#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include <glm/glm.hpp>

export module ghost_escape.world.effect;

export import ghost_escape.affiliate.sprite_anim;
export import ghost_escape.core;

export namespace pyc::sdl3 {

class Effect : public ObjectWorld {
public:
    static Effect* CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position,
                                float scale = 1.F, std::unique_ptr<ObjectWorld> next = nullptr);

    virtual void clean() override;

    virtual void update(std::chrono::duration<float> delta) override;

    void setNext(std::unique_ptr<ObjectWorld> next) { next_ = std::move(next); }

private:
    void checkFinish();

private:
    SpriteAnim* sprite_{};
    std::unique_ptr<ObjectWorld> next_{};
};

}  // namespace pyc::sdl3
