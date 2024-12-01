#include <string_view>

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <fmt/base.h>

#define ASSET_PATH "sdl2/demo/assets/"
#define ASSET(filename) ("sdl2/demo/assets/" filename)

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fmt::println("SDL_Init: {}", SDL_GetError());
    }
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    auto window =
        SDL_CreateWindow("你好 SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    auto render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    auto surface_img = IMG_Load(ASSET("background.png"));
    auto texture_img = SDL_CreateTextureFromSurface(render, surface_img);
    SDL_Rect rect_img{0, 0, 300, 300};

    auto font = TTF_OpenFont(ASSET("IPix.ttf"), 32);
    SDL_Color color{255, 255, 255, 255};
    auto surface_text = TTF_RenderUTF8_Blended(font, "你好，世界", color);
    auto texture_text = SDL_CreateTextureFromSurface(render, surface_text);
    SDL_Rect rect_text{0, 0, surface_text->w, surface_text->h};

    auto music = Mix_LoadMUS(ASSET("bgm.mp3"));
    Mix_FadeInMusic(music, -1, 1500);

    constexpr int kFPS = 60;
    bool is_quit = false;
    SDL_Event event;
    SDL_Point position_cursor{1280, 720};

    Uint64 time = SDL_GetTicks64();
    while (!is_quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_quit = true;
            } else if (event.type == SDL_MOUSEMOTION) {
                position_cursor.x = event.motion.x;
                position_cursor.y = event.motion.y;
            }
        }

        // 处理数据
        rect_img.x = position_cursor.x;
        rect_img.y = position_cursor.y;

        rect_text.x = position_cursor.x;
        rect_text.y = position_cursor.y;

        // 清空渲染器
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderClear(render);

        // 绘制图片
        SDL_RenderCopy(render, texture_img, nullptr, &rect_img);
        filledCircleRGBA(render, position_cursor.x, position_cursor.y, 50, 255, 0, 0, 125);
        SDL_RenderCopy(render, texture_text, nullptr, &rect_text);

        // 渲染
        SDL_RenderPresent(render);

        Uint64 elapse = SDL_GetTicks64() - time;
        auto delay = 1000.0 / kFPS - elapse;
        if (delay > 0) {
            SDL_Delay(delay);
        }
        time = SDL_GetTicks64();
        fmt::println("elapse: {} ms delay: {} ms", elapse, delay);
    }

    // 开始清理工作

    // 停止音乐并释放音乐对象
    Mix_FadeOutMusic(500);  // 在500毫秒内淡出音乐
    Mix_FreeMusic(music);   // 释放音乐资源

    // 销毁文本相关的纹理和表面
    SDL_DestroyTexture(texture_text);  // 释放文本纹理
    SDL_FreeSurface(surface_text);     // 释放渲染文本使用的表面资源

    // 关闭字体
    TTF_CloseFont(font);  // 关闭打开的字体文件

    // 销毁图片相关的纹理和表面
    SDL_DestroyTexture(texture_img);  // 释放图片纹理
    SDL_FreeSurface(surface_img);     // 释放加载图片使用的表面资源

    // 销毁渲染器和窗口
    SDL_DestroyRenderer(render);  // 销毁创建的渲染器
    SDL_DestroyWindow(window);    // 销毁创建的窗口

    // 退出子系统
    Mix_Quit();  // 退出SDL_mixer子系统
    IMG_Quit();  // 退出SDL_image子系统
    TTF_Quit();  // 退出SDL_ttf子系统

    // 退出SDL
    SDL_Quit();  // 退出SDL

    // 结束清理工作

    return 0;
}