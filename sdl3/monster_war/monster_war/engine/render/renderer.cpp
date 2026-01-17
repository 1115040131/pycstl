#include "monster_war/engine/render/renderer.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/render/camera.h"
#include "monster_war/engine/render/image.h"
#include "monster_war/engine/resource/resource_manager.h"

namespace pyc::monster_war {

using namespace entt::literals;

Renderer::Renderer(SDL_Renderer* sdl_renderer, ResourceManager* resource_manager)
    : sdl_renderer_(sdl_renderer), resource_manager_(resource_manager) {
    spdlog::trace("构造 Renderer...");
    if (!sdl_renderer_) {
        throw std::runtime_error("Renderer 构造失败: 提供的 SDL_Renderer 指针为空。");
    }
    if (!resource_manager_) {
        throw std::runtime_error("Renderer 构造失败: 提供的 ResourceManager 指针为空。");
    }
    setDrawColor(0, 0, 0, 255);
    spdlog::trace("Renderer 构造成功。");
}

void Renderer::drawSprite(const Camera& camera, const Sprite& sprite, const glm::vec2& position,
                          const glm::vec2& size, const float rotation, const FColor& color) {
    auto texture = resource_manager_->getTexture(sprite.texture_id_, sprite.texture_path_);
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.texture_id_);
        return;
    }

    // 应用相机变换
    auto screen_pos = camera.worldToScreen(position);

    // 计算目标矩形，注意 position 是精灵的左上角坐标
    SDL_FRect dest_rect = {
        screen_pos.x,
        screen_pos.y,
        size.x,
        size.y,
    };

    if (!isRectInViewport(camera, dest_rect)) {
        // spdlog::debug("精灵超出视口范围, ID: {}", image.getTextureId());
        return;
    }

    SDL_FRect src_rect = {
        sprite.src_rect_.position.x,
        sprite.src_rect_.position.y,
        sprite.src_rect_.size.x,
        sprite.src_rect_.size.y,
    };

    // 设置调整颜色与透明度
    SDL_SetTextureColorModFloat(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaModFloat(texture, color.a);

    // 执行绘制(默认旋转中心为精灵的中心点)
    if (!SDL_RenderTextureRotated(sdl_renderer_, texture, &src_rect, &dest_rect, rotation, nullptr,
                                  sprite.is_flipped_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染旋转纹理失败 (ID: {}): {}", sprite.texture_id_, SDL_GetError());
    }
}

void Renderer::drawFilledCircle(const Camera& camera, const glm::vec2& position, const float radius,
                                const FColor& color) {
    // 获取引擎自带的圆形纹理
    auto circle_texture = resource_manager_->getTexture("assets/textures/UI/circle.png"_hs);
    if (!circle_texture) {
        spdlog::error("无法获取引擎自带的圆形纹理。");
        return;
    }
    auto screen_position = camera.worldToScreen(position);
    // 设置颜色和透明度
    SDL_SetTextureColorModFloat(circle_texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaModFloat(circle_texture, color.a);
    // 绘制
    SDL_FRect dest_rect = {screen_position.x - radius, screen_position.y - radius, radius * 2, radius * 2};
    if (!SDL_RenderTextureRotated(sdl_renderer_, circle_texture, nullptr, &dest_rect, 0.0, nullptr,
                                  SDL_FLIP_NONE)) {
        spdlog::error("绘制填充圆形失败：{}", SDL_GetError());
    }
}

void Renderer::drawFilledRect(const Camera& camera, const glm::vec2& position, const glm::vec2& size,
                              const FColor& color) {
    // 应用相机变换
    auto screen_position = camera.worldToScreen(position);
    // 创建目标矩形
    SDL_FRect dest_rect = {screen_position.x, screen_position.y, size.x, size.y};
    // 设置颜色并绘制
    setDrawColorFloat(color.r, color.g, color.b, color.a);
    if (!SDL_RenderFillRect(sdl_renderer_, &dest_rect)) {
        spdlog::error("绘制填充矩形失败：{}", SDL_GetError());
    }
    // 恢复默认颜色
    setDrawColorFloat(0, 0, 0, 1.0f);
}

void Renderer::drawRect(const Camera& camera, const glm::vec2& position, const glm::vec2& size,
                        const FColor& color, const int thickness) {
    // 应用相机变换
    auto screen_position = camera.worldToScreen(position);
    // 创建目标矩形
    SDL_FRect dest_rect = {screen_position.x, screen_position.y, size.x, size.y};
    // 设置颜色并绘制
    setDrawColorFloat(color.r, color.g, color.b, color.a);
    for (int i = 0; i < thickness; i++) {
        if (!SDL_RenderRect(sdl_renderer_, &dest_rect)) {
            spdlog::error("绘制矩形边框失败：{}", SDL_GetError());
        }
        dest_rect.x += 1;
        dest_rect.y += 1;
        dest_rect.w -= 2;
        dest_rect.h -= 2;
    }
    // 恢复默认颜色
    setDrawColorFloat(0, 0, 0, 1.0f);
}

void Renderer::drawUIImage(const Image& image, const glm::vec2& position, std::optional<glm::vec2> size) {
    auto texture = resource_manager_->getTexture(image.getTextureId(), image.getTexturePath());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", image.getTextureId());
        return;
    }

    auto src_rect = getImageSrcRect(image);
    if (!src_rect) {
        spdlog::error("无法获取精灵的源矩形, ID: {}", image.getTextureId());
        return;
    }

    auto dest_rect = size ? SDL_FRect{position.x, position.y, size->x, size->y}
                          : SDL_FRect{position.x, position.y, src_rect->w, src_rect->h};

    if (!SDL_RenderTextureRotated(sdl_renderer_, texture, &src_rect.value(), &dest_rect, 0.0, nullptr,
                                  image.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染 UI Image 失败 (ID: {}): {}", image.getTextureId(), SDL_GetError());
    }
}

void Renderer::drawUIFilledRect(const Rect& rect, const FColor& color) {
    setDrawColorFloat(color.r, color.g, color.b, color.a);
    SDL_FRect sdl_rect{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
    if (!SDL_RenderFillRect(sdl_renderer_, &sdl_rect)) {
        spdlog::error("渲染填充矩形失败: {}", SDL_GetError());
    }
    setDrawColor(0, 0, 0, 1.0f);
}

void Renderer::present() { SDL_RenderPresent(sdl_renderer_); }

void Renderer::clearScreen() {
    setDrawColorFloat(background_color_.r, background_color_.g, background_color_.b, background_color_.a);
    if (!SDL_RenderClear(sdl_renderer_)) {
        spdlog::error("清除渲染器失败：{}", SDL_GetError());
    }
}

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!SDL_SetRenderDrawColor(sdl_renderer_, r, g, b, a)) {
        spdlog::error("设置渲染绘制颜色失败：{}", SDL_GetError());
    }
}

void Renderer::setDrawColorFloat(float r, float g, float b, float a) {
    if (!SDL_SetRenderDrawColorFloat(sdl_renderer_, r, g, b, a)) {
        spdlog::error("设置渲染绘制颜色失败：{}", SDL_GetError());
    }
}

std::optional<SDL_FRect> Renderer::getImageSrcRect(const Image& image) {
    auto texture = resource_manager_->getTexture(image.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", image.getTextureId());
        return std::nullopt;
    }

    auto source_rect = image.getSourceRect();
    if (source_rect) {  // 如果Image中存在指定rect，则判断尺寸是否有效
        if (source_rect->size.x <= 0 || source_rect->size.y <= 0) {
            spdlog::error("源矩形尺寸无效, ID: {}", image.getTextureId());
            return std::nullopt;
        }
        return SDL_FRect{
            source_rect->position.x,
            source_rect->position.y,
            source_rect->size.x,
            source_rect->size.y,
        };
    } else {  // 否则获取纹理尺寸并返回整个纹理大小
        SDL_FRect full_rest{};
        if (!SDL_GetTextureSize(texture, &full_rest.w, &full_rest.h)) {
            spdlog::error("无法获取纹理尺寸, ID: {}", image.getTextureId());
            return std::nullopt;
        }
        return full_rest;
    }
}

bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect& rect) {
    const auto& viewport_size = camera.getViewportSize();
    return rect.x + rect.w >= 0 && rect.x <= viewport_size.x && rect.y + rect.h >= 0 && rect.y <= viewport_size.y;
}

}  // namespace pyc::monster_war