#pragma once

#include <glm/glm.hpp>

#include "common/noncopyable.h"
#include "sunny_land/engine/render/sprite.h"
#include "sunny_land/engine/utils/math.h"

struct SDL_Renderer;

namespace pyc::sunny_land {

class Camera;
class ResourceManager;

/**
 * @brief 相机类负责管理相机位置和视口大小，并提供坐标转换功能。
 * 它还包含限制相机移动范围的边界。
 */
class Renderer final : Noncopyable {
public:
    /**
     * @brief 封装 SDL3 渲染操作
     *
     * 包装 SDL_Renderer 并提供清除屏幕、绘制精灵和呈现最终图像的方法。
     * 在构造时初始化。依赖于一个有效的 SDL_Renderer 和 ResourceManager。
     * 构造失败会抛出异常。
     */
    Renderer(SDL_Renderer* sdl_renderer, ResourceManager* resource_manager);

    /**
     * @brief 绘制一个精灵
     *
     * @param sprite 包含纹理ID、源矩形和翻转状态的 Sprite 对象。
     * @param position 世界坐标中的左上角位置。
     * @param scale 缩放因子。
     * @param angle 旋转角度（度）。
     */
    void drawSprite(const Camera& camera, const Sprite& sprite, const glm::vec2& position,
                    const glm::vec2& scale = {1.0f, 1.0f}, double angle = 0.0f);

    /**
     * @brief 绘制视差滚动背景
     *
     * @param sprite 包含纹理ID、源矩形和翻转状态的 Sprite 对象。
     * @param position 世界坐标。
     * @param scroll_factor 滚动因子。
     * @param scale 缩放因子。
     */
    void drawParallax(const Camera& camera, const Sprite& sprite, const glm::vec2& position,
                      const glm::vec2& scroll_factor, glm::bvec2 repeat = {true, true},
                      const glm::vec2& scale = {1.0f, 1.0f});

    /**
     * @brief 在屏幕坐标中直接渲染一个用于UI的Sprite对象。
     *
     * @param sprite 包含纹理ID、源矩形和翻转状态的Sprite对象。
     * @param position 屏幕坐标中的左上角位置。
     * @param size 可选：目标矩形的大小。如果为 std::nullopt，则使用Sprite的原始大小。
     */
    void drawUISprite(const Sprite& sprite, const glm::vec2& position,
                      std::optional<glm::vec2> size = std::nullopt);

    /**
     * @brief 绘制填充矩形
     *
     * @param rect 矩形区域
     * @param color 填充颜色
     */
    void drawUIFilledRect(const Rect& rect, const FColor& color);

    void present();      ///< @brief 更新屏幕，包装 SDL_RenderPresent 函数
    void clearScreen();  ///< @brief 清屏，包装 SDL_RenderClear 函数

    ///< @brief 设置绘制颜色，包装 SDL_SetRenderDrawColor 函数，使用 Uint8 类型
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    ///< @brief 设置绘制颜色，包装 SDL_SetRenderDrawColorFloat 函数，使用 float 类型
    void setDrawColorFloat(float r, float g, float b, float a = 1.0f);

    SDL_Renderer* getSDLRenderer() const { return sdl_renderer_; }  ///< @brief 获取底层的 SDL_Renderer 指针

private:
    ///< @brief 获取精灵的源矩形，用于具体绘制。出现错误则返回std::nullopt并跳过绘制
    std::optional<SDL_FRect> getSpriteSrcRect(const Sprite& sprite);
    ///< @brief 判断矩形是否在视口中，用于视口裁剪
    bool isRectInViewport(const Camera& camera, const SDL_FRect& rect);

private:
    SDL_Renderer* sdl_renderer_{};         ///< @brief 指向 SDL_Renderer 的非拥有指针
    ResourceManager* resource_manager_{};  ///< @brief 指向 ResourceManager 的非拥有指针
};

}  // namespace pyc::sunny_land
