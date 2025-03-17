#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/base.h>

#define ASSET_PATH "sdl3/demo/assets/"
#define ASSET(filename) ("sdl3/demo/assets/" filename)

int main() {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        fmt::println("SDL_Init: {}", SDL_GetError());
        return 1;
    }

    // 创建窗口和渲染器
    auto window = SDL_CreateWindow("你好 SDL", 1280, 720, 0);
    auto renderer = SDL_CreateRenderer(window, nullptr);

    // 加载图片
    auto texture_img = IMG_LoadTexture(renderer, ASSET("background.png"));

    if (!Mix_OpenAudio(0, nullptr)) {
        fmt::println("Mix_OpenAudio: {}", SDL_GetError());
        return 1;
    }

    // 读取音乐
    auto music = Mix_LoadMUS(ASSET("bgm.mp3"));
    Mix_PlayMusic(music, -1);

    // SDL_TTF 初始化
    if (!TTF_Init()) {
        fmt::println("TTF_Init: {}", SDL_GetError());
        return 1;
    }
    // 加载字体
    auto font = TTF_OpenFont(ASSET("IPix.ttf"), 32);
    // 创建文本纹理
    SDL_Color color{255, 255, 255, 255};
    auto surface_text = TTF_RenderText_Blended(font, "你好，世界", 0, color);
    auto texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);

    // SDL3 新的文本渲染方式
    auto text_engine = TTF_CreateRendererTextEngine(renderer);
    auto text = TTF_CreateText(text_engine, font, "SDL3 新的文本渲染方式", 0);

    bool is_running = true;
    while (is_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                is_running = false;
            }
        }

        // 清空渲染器
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 画一个图片
        SDL_FRect rect_img{0, 0, 300, 300};
        SDL_RenderTexture(renderer, texture_img, nullptr, &rect_img);

        // 画一个长方形
        SDL_FRect rect{100, 100, 200, 200};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        // 画文本
        SDL_FRect rect_text{300, 300, static_cast<float>(surface_text->w), static_cast<float>(surface_text->h)};
        SDL_RenderTexture(renderer, texture_text, nullptr, &rect_text);

        // 画新的文本
        TTF_DrawRendererText(text, 400, 400);

        SDL_RenderPresent(renderer);
    }

    // 开始清理工作
    // 清理图片资源
    SDL_DestroyTexture(texture_img);  // 释放图片纹理

    // 清理音乐资源
    Mix_FreeMusic(music);  // 释放音乐资源
    Mix_CloseAudio();      // 关闭音频子系统
    Mix_Quit();            // 退出SDL_mixer子系统

    // 清理文本资源
    SDL_DestroySurface(surface_text);  // 释放渲染文本使用的表面资源
    SDL_DestroyTexture(texture_text);  // 释放文本纹理
    TTF_CloseFont(font);               // 关闭打开的字体文件
    TTF_Quit();                        // 退出SDL_ttf子系统

    // 退出SDL
    SDL_DestroyRenderer(renderer);  // 释放渲染器
    SDL_DestroyWindow(window);      // 释放窗口
    SDL_Quit();                     // 退出SDL

    // 结束清理工作

    return 0;
}