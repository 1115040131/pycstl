#include "sunny_land/engine/physics/physics_engine.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/tilelayer_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/collision.h"

namespace pyc::sunny_land {

void PhysicsEngine::registerComponent(PhysicsComponent* component) {
    components_.push_back(component);
    spdlog::trace("物理组件注册完成。");
}

void PhysicsEngine::unregisterComponent(PhysicsComponent* component) {
    std::erase(components_, component);
    spdlog::trace("物理组件注销完成。");
}

void PhysicsEngine::registerCollisionLayer(TileLayerComponent* tile_layer) {
    tile_layer->setPhysicsEngine(this);
    tile_layers_.push_back(tile_layer);
    spdlog::trace("碰撞瓦片图层注册完成。");
}

void PhysicsEngine::unregisterCollisionLayer(TileLayerComponent* tile_layer) {
    std::erase(tile_layers_, tile_layer);
    spdlog::trace("碰撞瓦片图层注销完成。");
}

void PhysicsEngine::update(std::chrono::duration<float> delta_time) {
    for (auto physics : components_) {
        if (!physics || !physics->isEnabled()) {
            continue;
        }

        // 应用重力 (如果组件受重力影响)：F = g * m
        if (physics->isUseGravity()) {
            physics->addForce(gravity_ * physics->getMass());
        }

        // 更新速度： v += a * dt，其中 a = F / m
        physics->velocity_ += (physics->getForce() / physics->getMass()) * delta_time.count();
        physics->clearForce();

        // 处理瓦片层碰撞, 以及速度和位置的更新
        resolveTileCollisions(physics, delta_time);
    }
    // 处理对象间碰撞
    checkObjectCollisions();
}

void PhysicsEngine::checkObjectCollisions() {
    collision_pairs_.clear();

    for (size_t i = 0; i + 1 < components_.size(); ++i) {
        auto physics_a = components_[i];
        if (!physics_a || !physics_a->isEnabled()) {
            continue;
        }
        auto object_a = physics_a->getOwner();
        if (!object_a) {
            continue;
        }
        auto collider_a = object_a->getComponent<ColliderComponent>();
        if (!collider_a || !collider_a->isActive()) {
            continue;
        }

        for (size_t j = i + 1; j < components_.size(); ++j) {
            auto physics_b = components_[j];
            if (!physics_b || !physics_b->isEnabled()) {
                continue;
            }
            auto object_b = physics_b->getOwner();
            if (!object_b || object_a == object_b) {
                continue;
            }
            auto collider_b = object_b->getComponent<ColliderComponent>();
            if (!collider_b || !collider_b->isActive()) {
                continue;
            }

            if (checkCollision(*collider_a, *collider_b)) {
                // 如果是可移动物体与SOLID物体碰撞，则直接处理位置变化，不用记录碰撞对
                if (object_a->getTag() != "solid" && object_b->getTag() == "solid") {
                    resolveSolidObjectCollisions(object_a, object_b);
                } else if (object_a->getTag() == "solid" && object_b->getTag() != "solid") {
                    resolveSolidObjectCollisions(object_b, object_a);
                } else {
                    // 记录碰撞对
                    collision_pairs_.emplace_back(object_a, object_b);
                }
            }
        }
    }
}

void PhysicsEngine::resolveTileCollisions(PhysicsComponent* physics, std::chrono::duration<float> delta_time) {
    auto object = physics->getOwner();
    if (!object) {
        return;
    }
    auto transform = physics->getTransform();
    auto collider = object->getComponent<ColliderComponent>();
    if (!transform || !collider || !collider->isActive() || collider->isTrigger()) {
        return;
    }
    auto world_aabb = collider->getWorldAABB();
    if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) {
        return;
    }

    constexpr auto kTolerance = 1.0f;  // 检查右边缘和下边缘时，需要减1像素，否则会检查到下一行/列的瓦片
    auto ds = physics->velocity_ * delta_time.count();  // 计算物体在delta_time内的位移
    auto new_position = world_aabb.position + ds;       // 计算物体在delta_time后的新位置
    const auto& obj_position = world_aabb.position;
    const auto& obj_size = world_aabb.size;

    // 遍历所有注册的碰撞瓦片层
    for (auto tile_layer : tile_layers_) {
        if (!tile_layer) {
            continue;
        }
        const auto& tile_size = tile_layer->getTileSize();
        // 轴分离碰撞检测：先检查X方向是否有碰撞 (y方向使用初始值obj_pos.y)
        if (ds.x != 0.0f) {
            // 物体预期 x 坐标, ds.x > 0 取右边缘, 否则取左边缘
            auto tile_x = ds.x > 0.0f ? new_position.x + obj_size.x : new_position.x;
            // 物体预期上方 y 坐标所在瓦片类型
            auto tile_type_top = tile_layer->getTileTypeAtWorldPos({tile_x, obj_position.y});
            // 物体预期下方 y 坐标所在瓦片类型
            auto tile_type_bottom =
                tile_layer->getTileTypeAtWorldPos({tile_x, obj_position.y + obj_size.y - kTolerance});

            if (tile_type_top == TileType::SOLID || tile_type_bottom == TileType::SOLID) {
                // 撞墙了！速度归零，x方向移动到贴着墙的位置
                new_position.x = ds.x > 0.0f ? std::floor(tile_x / tile_size.x) * tile_size.x - obj_size.x
                                             : std::floor(tile_x / tile_size.x) * tile_size.x + tile_size.x;
                physics->velocity_.x = 0.0f;
            }
        }
        // 轴分离碰撞检测：再检查Y方向是否有碰撞 (x方向使用初始值obj_pos.x)
        if (ds.y != 0.0f) {
            // 物体预期 y 坐标, ds.y > 0 取上边缘, 否则取下边缘
            auto tile_y = ds.y > 0.0f ? new_position.y + obj_size.y : new_position.y;
            // 物体预期左侧 x 坐标所在瓦片类型
            auto tile_type_left = tile_layer->getTileTypeAtWorldPos({obj_position.x, tile_y});
            // 物体预期右侧 x 坐标所在瓦片类型
            auto tile_type_right =
                tile_layer->getTileTypeAtWorldPos({obj_position.x + obj_size.x - kTolerance, tile_y});

            if (tile_type_left == TileType::SOLID || tile_type_right == TileType::SOLID) {
                // 撞地板或天花板了！速度归零，y方向移动到贴着地板/天花板的位置
                new_position.y = ds.y > 0.0f ? std::floor(tile_y / tile_size.y) * tile_size.y - obj_size.y
                                             : std::floor(tile_y / tile_size.y) * tile_size.y + tile_size.y;
                physics->velocity_.y = 0.0f;
            }
        }
        // 更新物体位置，并限制最大速度
        // 使用translate方法，避免直接设置位置，因为碰撞盒可能有偏移量
        transform->translate(new_position - obj_position);
        physics->velocity_ = glm::clamp(physics->velocity_, -max_speed_, max_speed_);
    }
}

void PhysicsEngine::resolveSolidObjectCollisions(GameObject* move_obj, GameObject* solid_obj) {
    // 进入此函数前，已经检查了各个组件的有效性，因此直接进行计算
    auto move_tc = move_obj->getComponent<TransformComponent>();
    auto move_pc = move_obj->getComponent<PhysicsComponent>();
    auto move_cc = move_obj->getComponent<ColliderComponent>();
    auto solid_cc = solid_obj->getComponent<ColliderComponent>();

    // 这里只能获取期望位置，无法获取当前帧初始位置，因此无法进行轴分离碰撞检测
    /* 未来可以进行重构，让这里可以获取初始位置。但是我们展示另外一种处理方法 */
    auto move_aabb = move_cc->getWorldAABB();
    auto solid_aabb = solid_cc->getWorldAABB();

    // --- 使用最小平移向量解决碰撞问题 ---
    auto move_center = move_aabb.position + move_aabb.size / 2.0f;
    auto solid_center = solid_aabb.position + solid_aabb.size / 2.0f;
    // 计算两个包围盒的重叠部分
    auto overlap =
        glm::vec2(move_aabb.size / 2.0f + solid_aabb.size / 2.0f) - glm::abs(move_center - solid_center);
    if (overlap.x < 0.1f && overlap.y < 0.1f) {
        return;  // 如果重叠部分太小，则认为没有碰撞
    }

    if (overlap.x < overlap.y) {  // 如果重叠部分在x方向上更小，则认为碰撞发生在x方向上（推出x方向平移向量最小）
        if (move_center.x < solid_center.x) {
            // 移动物体在左边，让它贴着右边SOLID物体（相当于向左移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(-overlap.x, 0.0f));
            // 如果速度为正(向右移动)，则归零 （if判断不可少，否则可能出现错误吸附）
            if (move_pc->velocity_.x > 0.0f) {
                move_pc->velocity_.x = 0.0f;
            }
        } else {
            // 移动物体在右边，让它贴着左边SOLID物体（相当于向右移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(overlap.x, 0.0f));
            if (move_pc->velocity_.x < 0.0f) {
                move_pc->velocity_.x = 0.0f;
            }
        }
    } else {  // 重叠部分在y方向上更小，则认为碰撞发生在y方向上（推出y方向平移向量最小）
        if (move_center.y < solid_center.y) {
            // 移动物体在上面，让它贴着下面SOLID物体（相当于向上移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, -overlap.y));
            if (move_pc->velocity_.y > 0.0f) {
                move_pc->velocity_.y = 0.0f;
            }
        } else {
            // 移动物体在下面，让它贴着上面SOLID物体（相当于向下移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, overlap.y));
            if (move_pc->velocity_.y < 0.0f) {
                move_pc->velocity_.y = 0.0f;
            }
        }
    }
}

}  // namespace pyc::sunny_land