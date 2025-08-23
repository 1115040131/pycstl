#include "sunny_land/engine/render/camera.h"

#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

Camera::Camera(glm::vec2 viewport_size, glm::vec2 position, std::optional<Rect> limit_bounds)
    : viewport_size_(std::move(viewport_size)),
      position_(std::move(position)),
      limit_bounds_(std::move(limit_bounds)) {
    spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
}

void Camera::update([[maybe_unused]] std::chrono::duration<float> delta_time) {}

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

const glm::vec2& Camera::getViewportSize() const { return viewport_size_; }

const glm::vec2& Camera::getPosition() const { return position_; }

const std::optional<Rect>& Camera::getLimitBounds() const { return limit_bounds_; }

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