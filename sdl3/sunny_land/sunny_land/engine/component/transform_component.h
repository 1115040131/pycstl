#pragma once

#include <glm/glm.hpp>

#include "sunny_land/engine/component/component.h"

namespace pyc::sunny_land {

class TransformComponent final : public Component {
    friend class GameObject;

public:
    /**
     * @brief 构造函数
     * @param position 位置
     * @param scale 缩放
     * @param rotation 旋转
     */
    TransformComponent(glm::vec2 position = {0.0f, 0.0f}, glm::vec2 scale = {1.0f, 1.0f}, float rotation = 0.0f)
        : position_(std::move(position)), scale_(std::move(scale)), rotation_(rotation) {}

    ~TransformComponent() override = default;

    // Getters and setters
    const glm::vec2& getPosition() const { return position_; }                 ///< @brief 获取位置
    const glm::vec2& getScale() const { return scale_; }                       ///< @brief 获取缩放
    float getRotation() const { return rotation_; }                            ///< @brief 获取旋转
    void setPosition(glm::vec2 position) { position_ = std::move(position); }  ///< @brief 设置位置
    void setScale(glm::vec2 scale);  ///< @brief 设置缩放，应用缩放时应同步更新Sprite偏移量
    void setRotation(float rotation) { rotation_ = rotation; }  ///< @brief 设置旋转角度

    void translate(const glm::vec2& offset) { position_ += offset; }  ///< @brief 平移

private:
    glm::vec2 position_ = {0.0f, 0.0f};  ///< @brief 位置
    glm::vec2 scale_ = {1.0f, 1.0f};     ///< @brief 缩放
    float rotation_ = 0.0f;              ///< @brief 角度制，单位：度
};

}  // namespace pyc::sunny_land