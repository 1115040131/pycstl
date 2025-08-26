#pragma once

#include <glm/glm.hpp>

namespace pyc::sunny_land {

/**
 * @brief 碰撞器的抽象基类。
 * 所有具体的碰撞器都应继承此类。
 */
class Collider {
public:
    enum class Type {
        NONE,
        AABB,
        CIRCLE,
    };

    virtual ~Collider() = default;
    virtual Type getType() const = 0;  ///< @brief 获取碰撞器的类型。

    ///< @brief 设置最小包围盒的尺寸（宽度和高度）。
    void setAABBSize(glm::vec2 size) { aabb_size_ = std::move(size); }
    const glm::vec2& getAABBSize() const { return aabb_size_; }  ///< @brief 获取最小包围盒的尺寸（宽度和高度）。

protected:
    glm::vec2 aabb_size_{};  ///< @brief 覆盖Collider的最小包围盒的尺寸（宽度和高度）。
};

/**
 * @brief 轴对齐包围盒 (Axis-Aligned Bounding Box) 碰撞器。
 */
class AABBCollider final : public Collider {
public:
    /**
     * @brief 构造函数。
     * @param size 包围盒的宽度和高度。
     */
    explicit AABBCollider(glm::vec2 size) { setAABBSize(std::move(size)); }

    // --- Getters and Setters ---
    Type getType() const override { return Type::AABB; }
};

/**
 * @brief 圆形碰撞器。
 */
class CircleCollider final : public Collider {
public:
    /**
     * @brief 构造函数。
     * @param radius 圆的半径。
     */
    explicit CircleCollider(float radius) : radius_(radius) {
        setAABBSize(glm::vec2(radius * 2.0f, radius * 2.0f));
    }

    // --- Getters and Setters ---
    Type getType() const override { return Type::CIRCLE; }
    float getRadius() const { return radius_; }
    void setRadius(float radius) { radius_ = radius; }

private:
    float radius_{};  ///< @brief 圆的半径。
};

}  // namespace pyc::sunny_land