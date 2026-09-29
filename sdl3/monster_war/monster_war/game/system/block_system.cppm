module;

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

export module monster_war.game.system.block_system;

export namespace pyc::monster_war {

/**
 * @brief 阻挡系统
 * 用于判断敌人是否被阻挡，并更新阻挡相关组件。
 */
class BlockSystem {
public:
    void update(entt::registry& registry, entt::dispatcher& dispatcher);
};

}  // namespace pyc::monster_war
