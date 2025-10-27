#include "sunny_land/engine/render/renderer.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/resource/resource_manager.h"

namespace pyc::sunny_land {

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
                          const glm::vec2& scale, double angle) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect) {
        spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
        return;
    }

    // 应用相机变换
    auto screen_pos = camera.worldToScreen(position);

    // 计算目标矩形，注意 position 是精灵的左上角坐标
    SDL_FRect dest_rect = {
        screen_pos.x,
        screen_pos.y,
        src_rect->w * scale.x,
        src_rect->h * scale.y,
    };

    if (!isRectInViewport(camera, dest_rect)) {
        // spdlog::debug("精灵超出视口范围, ID: {}", sprite.getTextureId());
        return;
    }

    // 执行绘制(默认旋转中心为精灵的中心点)
    if (!SDL_RenderTextureRotated(sdl_renderer_, texture, &src_rect.value(), &dest_rect, angle, nullptr,
                                  sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染旋转纹理失败 (ID: {}): {}", sprite.getTextureId(), SDL_GetError());
    }
}

void Renderer::drawParallax(const Camera& camera, const Sprite& sprite, const glm::vec2& position,
                            const glm::vec2& scroll_factor, glm::bvec2 repeat, const glm::vec2& scale) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect) {
        spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
        return;
    }

    // 应用相机变换
    auto screen_pos = camera.worldToScreenWithParallax(position, scroll_factor);

    // 计算缩放后的纹理尺寸
    float scaled_tex_w = src_rect->w * scale.x;
    float scaled_tex_h = src_rect->h * scale.y;

    glm::vec2 start{};
    glm::vec2 stop{};
    auto viewport_size = camera.getViewportSize();

    if (repeat.x) {
        start.x = glm::mod(screen_pos.x, scaled_tex_w) - scaled_tex_w;
        stop.x = viewport_size.x;
    } else {
        start.x = screen_pos.x;
        stop.x = glm::min(screen_pos.x + scaled_tex_w, viewport_size.x);
    }
    if (repeat.y) {
        start.y = glm::mod(screen_pos.y, scaled_tex_h) - scaled_tex_h;
        stop.y = viewport_size.y;
    } else {
        start.y = screen_pos.y;
        stop.y = glm::min(screen_pos.y + scaled_tex_h, viewport_size.y);
    }
    for (float y = start.y; y < stop.y; y += scaled_tex_h) {
        for (float x = start.x; x < stop.x; x += scaled_tex_w) {
            SDL_FRect dest_rect = {x, y, scaled_tex_w, scaled_tex_h};
            if (!SDL_RenderTexture(sdl_renderer_, texture, &src_rect.value(), &dest_rect)) {
                spdlog::error("渲染视差纹理失败 (ID: {}): {}", sprite.getTextureId(), SDL_GetError());
                return;
            }
        }
    }
}

void Renderer::drawUISprite(const Sprite& sprite, const glm::vec2& position, std::optional<glm::vec2> size) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect) {
        spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
        return;
    }

    auto dest_rect = size ? SDL_FRect{position.x, position.y, size->x, size->y}
                          : SDL_FRect{position.x, position.y, src_rect->w, src_rect->h};

    if (!SDL_RenderTextureRotated(sdl_renderer_, texture, &src_rect.value(), &dest_rect, 0.0, nullptr,
                                  sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染 UI Sprite 失败 (ID: {}): {}", sprite.getTextureId(), SDL_GetError());
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

std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite& sprite) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return std::nullopt;
    }

    auto source_rect = sprite.getSourceRect();
    if (source_rect) {  // 如果Sprite中存在指定rect，则判断尺寸是否有效
        if (source_rect->w <= 0 || source_rect->h <= 0) {
            spdlog::error("源矩形尺寸无效, ID: {}", sprite.getTextureId());
            return std::nullopt;
        }
        return source_rect;
    } else {  // 否则获取纹理尺寸并返回整个纹理大小
        SDL_FRect full_rest{};
        if (!SDL_GetTextureSize(texture, &full_rest.w, &full_rest.h)) {
            spdlog::error("无法获取纹理尺寸, ID: {}", sprite.getTextureId());
            return std::nullopt;
        }
        return full_rest;
    }
}

bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect& rect) {
    const auto& viewport_size = camera.getViewportSize();
    return rect.x + rect.w >= 0 && rect.x <= viewport_size.x && rect.y + rect.h >= 0 && rect.y <= viewport_size.y;
}

}  // namespace pyc::sunny_land