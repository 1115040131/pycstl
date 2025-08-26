#include "sunny_land/engine/physics/collision.h"

#include "sunny_land/engine/component/collider_component.h"

namespace pyc::sunny_land {

bool checkCollision(const ColliderComponent& a, const ColliderComponent& b) {
    // 获取两个碰撞盒及对应Transform信息
    auto a_collider = a.getCollider();
    auto b_collider = b.getCollider();
    auto a_rect = a.getWorldAABB();
    auto b_rect = b.getWorldAABB();

    // 先计算最小包围盒是否碰撞，如果没有碰撞，那一定是返回false (不考虑AABB的旋转)
    if (!checkRectOverlap(a_rect, b_rect)) {
        return false;
    }

    // --- 如果最小包围盒有碰撞，再进行更细致的判断 ---
    // AABB vs AABB, 直接返回真
    if (a_collider->getType() == Collider::Type::AABB && b_collider->getType() == Collider::Type::AABB) {
        return true;
    }  // Circle vs Circle: 判断两个圆心距离是否小于两个圆的半径之和
    else if (a_collider->getType() == Collider::Type::CIRCLE && b_collider->getType() == Collider::Type::CIRCLE) {
        auto a_center = a_rect.position + a_rect.size / 2.0f;
        auto b_center = b_rect.position + b_rect.size / 2.0f;
        auto a_radius = a_rect.size.x / 2.0f;
        auto b_radius = b_rect.size.x / 2.0f;
        return checkCircleOverlap(a_center, a_radius, b_center, b_radius);
    }  // AABB vs Circle: 判断圆心到AABB的最邻近点是否在圆内
    else if (a_collider->getType() == Collider::Type::AABB && b_collider->getType() == Collider::Type::CIRCLE) {
        auto b_center = b_rect.position + b_rect.size / 2.0f;
        auto b_radius = b_rect.size.x / 2.0f;
        // 计算圆心到AABB的最邻近点
        auto nearest_point = glm::clamp(b_center, a_rect.position, a_rect.position + a_rect.size);
        return checkPointInCircle(nearest_point, b_center, b_radius);
    } else if (a_collider->getType() == Collider::Type::CIRCLE && b_collider->getType() == Collider::Type::AABB) {
        auto a_center = a_rect.position + a_rect.size / 2.0f;
        auto a_radius = a_rect.size.x / 2.0f;
        // 计算圆心到AABB的最邻近点
        auto nearest_point = glm::clamp(a_center, b_rect.position, b_rect.position + b_rect.size);
        return checkPointInCircle(nearest_point, a_center, a_radius);
    }

    return false;
}

bool checkCircleOverlap(const glm::vec2& a_center, const float a_radius, const glm::vec2& b_center,
                        const float b_radius) {
    return glm::length(a_center - b_center) < (a_radius + b_radius);
}

bool checkAABBOverlap(const glm::vec2& a_pos, const glm::vec2& a_size, const glm::vec2& b_pos,
                      const glm::vec2& b_size) {
    return a_pos.x + a_size.x > b_pos.x && a_pos.x < b_pos.x + b_size.x && a_pos.y + a_size.y > b_pos.y &&
           a_pos.y < b_pos.y + b_size.y;
}

bool checkRectOverlap(const Rect& a, const Rect& b) {
    return checkAABBOverlap(a.position, a.size, b.position, b.size);
}

bool checkPointInCircle(const glm::vec2& point, const glm::vec2& center, const float radius) {
    return glm::length(point - center) < radius;
}

}  // namespace pyc::sunny_land