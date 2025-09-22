#include "sunny_land/engine/render/camera.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/transform_component.h"

namespace pyc::sunny_land {

Camera::Camera(glm::vec2 viewport_size, glm::vec2 position, std::optional<Rect> limit_bounds)
    : viewport_size_(std::move(viewport_size)),
      position_(std::move(position)),
      limit_bounds_(std::move(limit_bounds)) {
    spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
}

void Camera::update(std::chrono::duration<float> delta_time) {
    if (!target_) {
        return;
    }
    auto target_pos = target_->getPosition();
    auto desired_position = target_pos - viewport_size_ * 0.5f;

    // 计算当前位置与目标位置的距离
    auto distance_ = glm::distance(position_, desired_position);
    constexpr float SNAP_THRESHOLD = 1.0f;  // 设置一个距离阈值  (constexpr: 编译时常量，避免每次调用都计算)

    if (distance_ < SNAP_THRESHOLD) {
        // 如果距离小于阈值，直接吸附到目标位置
        position_ = desired_position;
    } else {
        // 否则，使用线性插值平滑移动   glm::mix(a,b,t): 在向量 a 和 b 之间进行插值，t 是插值因子，范围在0到1之间。
        // 公式: (b-a)*t + a;   t = 0 时结果为 a，t = 1 时结果为 b
        position_ = glm::mix(position_, desired_position, smooth_speed_ * delta_time.count());
        position_ = glm::vec2(glm::round(position_.x),
                              glm::round(position_.y));  // 四舍五入到整数,省略的话偶尔会出现画面割裂
    }

    clampPosition();
}

void Camera::move(const glm::vec2& offset) {
    position_ += offset;
    clampPosition();
}

glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const { return world_pos - position_; }

glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const {
    return world_pos - position_ * scroll_factor;
}

glm::vec2 Camera::screenToWorld(const glm::vec2& screen_pos) const { return screen_pos + position_; }

void Camera::setPosition(const glm::vec2& position) {
    position_ = position;
    clampPosition();
}

void Camera::setLimitBounds(std::optional<Rect> limit_bounds) {
    limit_bounds_ = std::move(limit_bounds);
    clampPosition();
}

void Camera::clampPosition() {
    if (limit_bounds_ && limit_bounds_->size.x > 0 && limit_bounds_->size.y > 0) {
        glm::vec2 min_cam_pos = limit_bounds_->position;
        glm::vec2 max_cam_pos = limit_bounds_->position + limit_bounds_->size - viewport_size_;

        // 确保 max_cam_pos 不小于 min_cam_pos (视口可能比世界还大)
        max_cam_pos.x = std::max(max_cam_pos.x, min_cam_pos.x);
        max_cam_pos.y = std::max(max_cam_pos.y, min_cam_pos.y);

        position_ = glm::clamp(position_, min_cam_pos, max_cam_pos);
    }
}

}  // namespace pyc::sunny_land