module;

#include <glm/vec2.hpp>

export module ghost_escape.core:collider;

export import :affiliate;

export namespace pyc::sdl3 {

class ObjectWorld;

class Collider : public ObjectAffiliate {
public:
    enum class Type {
        kCircle,  // 对应 size_.x 为直径
        kRectange,
    };

    static Collider* CreateAndSet(ObjectWorld* parent, glm::vec2 size, Type type = Type::kCircle,
                                  Anchor anchor = Anchor::kCenter);

    virtual void render() override;

    bool isColliding(const Collider& other) const;

private:
    Type type_{Type::kCircle};
};

}  // namespace pyc::sdl3
