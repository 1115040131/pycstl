#pragma once

#include <glm/glm.hpp>

#include "sunny_land/engine/component/component.h"
#include "sunny_land/engine/render/sprite.h"
#include "sunny_land/engine/utils/alignment.h"

namespace pyc::sunny_land {

class ResourceManager;
class TransformComponent;

class SpriteComponent final : public Component {
    friend class GameObject;
    friend class TransformComponent;

public:
    /**
     * @brief 构造函数
     * @param texture_id 纹理资源的标识符。
     * @param resource_manager 资源管理器指针。
     * @param alignment 初始对齐方式。
     * @param source_rect_opt 可选的源矩形。
     * @param is_flipped 初始翻转状态。
     */
    SpriteComponent(std::string_view texture_id, ResourceManager& resource_manager,
                    Alignment alignment = Alignment::NONE, std::optional<SDL_FRect> source_rect_opt = std::nullopt,
                    bool is_flipped = false);

    ~SpriteComponent() override = default;

    // Getters
    const Sprite& getSprite() const { return sprite_; }                       ///< @brief 获取精灵对象
    std::string_view getTextureId() const { return sprite_.getTextureId(); }  ///< @brief 获取纹理ID
    bool isFlipped() const { return sprite_.isFlipped(); }                    ///< @brief 获取是否翻转
    bool isHidden() const { return is_hidden_; }                              ///< @brief 获取是否隐藏
    const glm::vec2& getSpriteSize() const { return sprite_size_; }           ///< @brief 获取精灵尺寸
    const glm::vec2& getOffset() const { return offset_; }                    ///< @brief 获取偏移量
    Alignment getAlignment() const { return alignment_; }                     ///< @brief 获取对齐方式

    // Setters
    void setSpriteById(std::string_view texture_id,
                       std::optional<SDL_FRect> source_rect_opt = std::nullopt);  ///< @brief 设置精灵对象
    void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }                ///< @brief 设置是否翻转
    void setHidden(bool hidden) { is_hidden_ = hidden; }                          ///< @brief 设置是否隐藏
    void setSourceRect(std::optional<SDL_FRect> source_rect_opt);                 ///< @brief 设置源矩形
    void setAlignment(Alignment anchor);                                          ///< @brief 设置对齐方式

private:
    void updateSpriteSize();  ///< @brief 辅助函数，根据 sprite_ 的 source_rect_ 更新 sprite_size_

    void updateOffset();  ///< @brief 更新偏移量(根据当前的 alignment_ 和 sprite_size_ 计算 offset_)。

    // Component 虚函数覆盖
    void init() override;                                             ///< @brief 初始化函数需要覆盖
    void update(std::chrono::duration<double>, Context&) override {}  ///< @brief 更新函数留空
    void render(Context& context) override;                           ///< @brief 渲染函数需要覆盖

private:
    ResourceManager* resource_manager_{};        ///< @brief 保存资源管理器指针，用于获取纹理大小
    TransformComponent* transform_component_{};  ///< @brief 缓存 TransformComponent 指针（非必须）

    Sprite sprite_;                         ///< @brief 精灵对象
    Alignment alignment_{Alignment::NONE};  ///< @brief 对齐方式
    glm::vec2 sprite_size_{};               ///< @brief 精灵尺寸
    glm::vec2 offset_{};                    ///< @brief 偏移量
    bool is_hidden_ = false;                ///< @brief 是否隐藏（不渲染）
};

};  // namespace pyc::sunny_land