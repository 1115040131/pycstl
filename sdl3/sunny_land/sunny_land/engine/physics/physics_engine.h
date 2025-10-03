#pragma once

#include <chrono>
#include <vector>

#include <glm/glm.hpp>

#include "common/noncopyable.h"
#include "sunny_land/engine/utils/math.h"

namespace pyc::sunny_land {

class GameObject;
class PhysicsComponent;
class TileLayerComponent;
enum class TileType;

class PhysicsEngine : Noncopyable, std::enable_shared_from_this<PhysicsEngine> {
public:
    void registerComponent(PhysicsComponent* component);    ///< @brief 注册物理组件
    void unregisterComponent(PhysicsComponent* component);  ///< @brief 注销物理组件

    // 如果瓦片层需要进行碰撞检测则注册。（不需要则不必注册）
    ///< @brief 注册用于碰撞检测的 TileLayerComponent
    void registerCollisionLayer(TileLayerComponent* tile_layer);
    ///< @brief 注销用于碰撞检测的 TileLayerComponent
    void unregisterCollisionLayer(TileLayerComponent* tile_layer);

    void update(std::chrono::duration<float> delta_time);  ///< @brief 核心循环：更新所有注册的物理组件的状态

    // 设置器/获取器
    void setGravity(glm::vec2 gravity) { gravity_ = std::move(gravity); }  ///< @brief 设置全局重力加速度
    const glm::vec2& getGravity() const { return gravity_; }               ///< @brief 获取当前的全局重力加速度
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }          ///< @brief 设置最大速度
    float getMaxSpeed() const { return max_speed_; }                       ///< @brief 获取当前的最大速度
    void setWorldBounds(Rect world_bounds) { world_bounds_ = std::move(world_bounds); }  ///< @brief 设置世界边界
    const std::optional<Rect>& getWorldBounds() const { return world_bounds_; }          ///< @brief 获取世界边界

    /// @brief 获取本帧检测到的所有 GameObject 碰撞对。(此列表在每次 update 开始时清空)
    const std::vector<std::pair<GameObject*, GameObject*>>& getCollisionPairs() const { return collision_pairs_; };
    /// @brief 获取本帧检测到的所有瓦片触发事件。(此列表在每次 update 开始时清空)
    const std::vector<std::pair<GameObject*, TileType>>& getTileTriggerEvents() const {
        return tile_trigger_events_;
    };

private:
    void checkObjectCollisions();  ///< @brief 检测并处理对象之间的碰撞，并记录需要游戏逻辑处理的碰撞对。

    /// @brief 检测并处理游戏对象和瓦片层之间的碰撞。
    void resolveTileCollisions(PhysicsComponent* physics, std::chrono::duration<float> delta_time);

    /// @brief 处理可移动物体与SOLID物体的碰撞。
    void resolveSolidObjectCollisions(GameObject* move_obj, GameObject* solid_obj);

    void applyWorldBounds(PhysicsComponent* pc);  ///< @brief 应用世界边界，限制物体移动范围

    /**
     * @brief 根据瓦片类型和指定宽度x坐标，计算瓦片上对应y坐标。
     * @param width 从瓦片左侧起算的宽度。
     * @param type 瓦片类型。
     * @param tile_size 瓦片尺寸。
     * @return 瓦片上对应高度（从瓦片下侧起算）。
     */
    float getTileHeightAtWidth(float width, TileType type, glm::vec2 tile_size);

    /**
     * @brief 检测所有游戏对象与瓦片层的触发器类型瓦片碰撞，并记录触发事件。(位移处理完毕后再调用)
     */
    void checkTileTriggers();

private:
    std::vector<PhysicsComponent*> components_;     ///< @brief 注册的物理组件容器，非拥有指针
    std::vector<TileLayerComponent*> tile_layers_;  ///< @brief 注册的碰撞瓦片图层容器
    glm::vec2 gravity_ = {0.0f, 980.0f};            ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
    float max_speed_ = 500.0f;                      ///< @brief 最大速度 (像素/秒)
    std::optional<Rect> world_bounds_;              ///< @brief 世界边界，用于限制物体移动范围

    /// @brief 存储本帧发生的 GameObject 碰撞对 （每次 update 开始时清空）
    std::vector<std::pair<GameObject*, GameObject*>> collision_pairs_;
    /// @brief 存储本帧发生的 GameObject 碰撞对 （每次 update 开始时清空）
    std::vector<std::pair<GameObject*, TileType>> tile_trigger_events_;
};

}  // namespace pyc::sunny_land