#include "chicken_evil/engine.h"

#include <chrono>
#include <random>
#include <ranges>
#include <thread>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <fmt/base.h>

#include "chicken_evil/gameplay.h"
#include "chicken_evil/resource_mgr.h"

#define ASSET_PATH "sdl2/chicken_evil/assets/"
#define ASSET(filename) (ASSET_PATH filename)

namespace pyc {
namespace sdl2 {

using namespace std::chrono_literals;

static Gameplay gameplay;

void Engine::init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fmt::println("SDL_Init: {}", SDL_GetError());
    }
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(32);

    window_ =
        SDL_CreateWindow("生化危鸡", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

    SDL_ShowCursor(SDL_DISABLE);

    load_resources();

    camera_ = std::make_unique<Camera>(renderer_);

    std::default_random_engine e;
    e.seed(time(0));

    gameplay.timer_generate.set_one_shot(false);
    gameplay.timer_generate.set_wait_time(1.5s);
    gameplay.timer_generate.set_on_timeout([&]() {
        for (int i = 0; i < gameplay.num_per_gen; i++) {
            auto val = e() % 100;
            if (val < 50) {
                gameplay.chicken.push_back(std::make_unique<ChickenFast>());
            } else if (val < 80) {
                gameplay.chicken.push_back(std::make_unique<ChickenMedium>());
            } else {
                gameplay.chicken.push_back(std::make_unique<ChickenSlow>());
            }
        }
    });

    gameplay.timer_increase_num_per_gen.set_one_shot(false);
    gameplay.timer_increase_num_per_gen.set_wait_time(8s);
    gameplay.timer_increase_num_per_gen.set_on_timeout([]() { gameplay.num_per_gen++; });

    gameplay.animation_barrel_fire.set_loop(false);
    gameplay.animation_barrel_fire.set_interval(0.04s);
    gameplay.animation_barrel_fire.set_center(gameplay.center_barrel);
    gameplay.animation_barrel_fire.add_frame(ResourceMgr::GetInstance().AtlasBarrelFire());
    gameplay.animation_barrel_fire.set_on_finished([]() { gameplay.is_cool_down = true; });
    gameplay.animation_barrel_fire.set_position({718, 610});

    Mix_PlayMusic(ResourceMgr::GetInstance().MusicBgm(), -1);
}

void Engine::mainloop() {
    SDL_Event event;

    auto duration = 1s / 144;

    auto last_time = std::chrono::high_resolution_clock::now();
    while (!gameplay.is_quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameplay.is_quit = true;
                    break;
                case SDL_MOUSEMOTION: {
                    gameplay.pos_crosshair = {event.motion.x, event.motion.y};
                    auto direction = gameplay.pos_crosshair - gameplay.pos_battery;
                    gameplay.angle_barrel = std::atan2(direction.y(), direction.x()) * 180 / M_PI;
                } break;
                case SDL_MOUSEBUTTONDOWN:
                    gameplay.is_fire_key_down = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    gameplay.is_fire_key_down = false;
                    break;
                default:
                    break;
            }
        }

        auto frame_start = std::chrono::high_resolution_clock::now();
        auto delta = frame_start - last_time;

        on_update(delta);

        on_render(*camera_);

        SDL_RenderPresent(renderer_);

        last_time = frame_start;
        auto delay = duration - (std::chrono::high_resolution_clock::now() - frame_start);
        if (delay > 0s) {
            std::this_thread::sleep_for(delay);
        }
    }
}

void Engine::deinit() {
    unload_resources();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);

    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Engine::load_resources() {
    auto& resource_mgr = ResourceMgr::GetInstance();

    resource_mgr.tex_heart_ = IMG_LoadTexture(renderer_, ASSET("heart.png"));
    resource_mgr.tex_bullet_ = IMG_LoadTexture(renderer_, ASSET("bullet.png"));
    resource_mgr.tex_battery_ = IMG_LoadTexture(renderer_, ASSET("battery.png"));
    resource_mgr.tex_crosshair_ = IMG_LoadTexture(renderer_, ASSET("crosshair.png"));
    resource_mgr.tex_background_ = IMG_LoadTexture(renderer_, ASSET("background.png"));
    resource_mgr.tex_barrel_idle_ = IMG_LoadTexture(renderer_, ASSET("barrel_idle.png"));

    resource_mgr.atlas_barrel_fire_.load(renderer_, ASSET("barrel_fire_{}.png"), 3);
    resource_mgr.atlas_chicken_fast_.load(renderer_, ASSET("chicken_fast_{}.png"), 4);
    resource_mgr.atlas_chicken_medium_.load(renderer_, ASSET("chicken_medium_{}.png"), 6);
    resource_mgr.atlas_chicken_slow_.load(renderer_, ASSET("chicken_slow_{}.png"), 8);
    resource_mgr.atlas_explosion_.load(renderer_, ASSET("explosion_{}.png"), 5);

    resource_mgr.music_bgm_ = Mix_LoadMUS(ASSET("bgm.mp3"));
    resource_mgr.music_loss_ = Mix_LoadMUS(ASSET("loss.mp3"));

    resource_mgr.sound_hurt_ = Mix_LoadWAV(ASSET("hurt.wav"));
    resource_mgr.sound_fire_1_ = Mix_LoadWAV(ASSET("fire_1.wav"));
    resource_mgr.sound_fire_2_ = Mix_LoadWAV(ASSET("fire_2.wav"));
    resource_mgr.sound_fire_3_ = Mix_LoadWAV(ASSET("fire_3.wav"));
    resource_mgr.sound_explosion_ = Mix_LoadWAV(ASSET("explosion.wav"));

    resource_mgr.font_ = TTF_OpenFont(ASSET("IPix.ttf"), 28);
}

void Engine::unload_resources() {
    auto& resource_mgr = ResourceMgr::GetInstance();

    SDL_DestroyTexture(resource_mgr.tex_heart_);
    SDL_DestroyTexture(resource_mgr.tex_bullet_);
    SDL_DestroyTexture(resource_mgr.tex_battery_);
    SDL_DestroyTexture(resource_mgr.tex_crosshair_);
    SDL_DestroyTexture(resource_mgr.tex_background_);
    SDL_DestroyTexture(resource_mgr.tex_barrel_idle_);

    Mix_FreeMusic(resource_mgr.music_bgm_);
    Mix_FreeMusic(resource_mgr.music_loss_);

    Mix_FreeChunk(resource_mgr.sound_hurt_);
    Mix_FreeChunk(resource_mgr.sound_fire_1_);
    Mix_FreeChunk(resource_mgr.sound_fire_2_);
    Mix_FreeChunk(resource_mgr.sound_fire_3_);
    Mix_FreeChunk(resource_mgr.sound_explosion_);
}

void Engine::on_update(std::chrono::duration<double> delta) {
    gameplay.timer_generate.on_update(delta);
    gameplay.timer_increase_num_per_gen.on_update(delta);

    // 更新子弹
    for (auto& bullet : gameplay.bullets) {
        bullet.on_update(delta);
    }

    // 更新鸡
    for (auto& chick : gameplay.chicken) {
        chick->on_update(delta);

        for (auto& bullet : gameplay.bullets) {
            if (!chick->check_alive()) {
                break;
            }

            if (bullet.can_remove()) {
                continue;
            }

            auto pos_delta = chick->position() - bullet.position();
            if (pos_delta.x() > -15 && pos_delta.x() < 15 && pos_delta.y() > -20 && pos_delta.y() < 20) {
                chick->on_hurt();
                bullet.on_hit();
                gameplay.score++;
            }
        }

        if (chick->check_alive() && chick->position().y() > 720) {
            chick->make_invalid();
            Mix_PlayChannel(-1, ResourceMgr::GetInstance().SoundHurt(), 0);
            gameplay.hp--;
        }
    }
    // 移除无效子弹
    std::erase_if(gameplay.bullets, [](const Bullet& bullet) { return bullet.can_remove(); });

    // 移除无效鸡
    std::erase_if(gameplay.chicken, [](const std::unique_ptr<Chicken>& chick) { return chick->can_remove(); });

    // 对场景中的鸡按竖直位置排序
    std::ranges::sort(gameplay.chicken,
                      [](const std::unique_ptr<Chicken>& lhs, const std::unique_ptr<Chicken>& rhs) {
                          return lhs->position().y() < rhs->position().y();
                      });

    // 处理正在开火
    if (!gameplay.is_cool_down) {
        camera_->shake(3.0, 0.1s);
        gameplay.animation_barrel_fire.on_update(delta);
    }

    // 处理开火瞬间
    if (gameplay.is_cool_down && gameplay.is_fire_key_down) {
        gameplay.animation_barrel_fire.reset();
        gameplay.is_cool_down = false;

        constexpr double length_barrel = 105;                  // 炮管长度
        static const Eigen::Vector2d barrel_center{640, 610};  // 炮管锚点中心

        gameplay.bullets.emplace_back(gameplay.angle_barrel);
        auto& bullet = gameplay.bullets.back();
        double angle_offset = gameplay.angle_barrel + (std::rand() % 30 - 15);  // 在 30 度范围内随机偏移
        double radians = angle_offset * M_PI / 180;
        Eigen::Vector2d direction{std::cos(radians), std::sin(radians)};
        bullet.set_position(barrel_center + direction * length_barrel);

        switch (std::rand() % 3) {
            case 0:
                Mix_PlayChannel(-1, ResourceMgr::GetInstance().SoundFire1(), 0);
                break;
            case 1:
                Mix_PlayChannel(-1, ResourceMgr::GetInstance().SoundFire2(), 0);
                break;
            case 2:
                Mix_PlayChannel(-1, ResourceMgr::GetInstance().SoundFire3(), 0);
                break;
            default:
                break;
        }
    }

    // 更新摄像机位置
    camera_->on_update(delta);

    // 检查游戏是否结束
    if (gameplay.hp <= 0) {
        gameplay.is_quit = true;
        Mix_HaltMusic();
        Mix_PlayMusic(ResourceMgr::GetInstance().MusicLoss(), 0);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "game over",
                                 fmt::format("final score: {}", gameplay.score).c_str(), nullptr);
    }
}

void Engine::on_render(const Camera& camera) {
    const auto& resource_mgr = ResourceMgr::GetInstance();
    // 绘制背景
    {
        int width;
        int height;
        SDL_QueryTexture(resource_mgr.TextureBackground(), nullptr, nullptr, &width, &height);
        SDL_FRect rect_background{
            (1280 - width) / 2.0f,
            (720 - height) / 2.0f,
            static_cast<float>(width),
            static_cast<float>(height),
        };
        camera.render_texture(resource_mgr.TextureBackground(), nullptr, &rect_background, 0, nullptr);
    }
    // 绘制鸡
    for (const auto& chick : gameplay.chicken) {
        chick->on_render(camera);
    }
    // 绘制子弹
    for (const auto& bullet : gameplay.bullets) {
        bullet.on_render(camera);
    }
    // 绘制炮台
    {
        // 底座
        int width_battery;
        int height_battery;
        SDL_QueryTexture(resource_mgr.TextureBattery(), nullptr, nullptr, &width_battery, &height_battery);
        SDL_FRect rect_battery{
            static_cast<float>(gameplay.pos_battery.x() - width_battery / 2.0),
            static_cast<float>(gameplay.pos_battery.y() - height_battery / 2.0f),
            static_cast<float>(width_battery),
            static_cast<float>(height_battery),
        };
        camera.render_texture(resource_mgr.TextureBattery(), nullptr, &rect_battery, 0, nullptr);

        // 炮管
        int width_barrel;
        int height_barrel;
        SDL_QueryTexture(resource_mgr.TextureBarrelIdle(), nullptr, nullptr, &width_barrel, &height_barrel);
        SDL_FRect rect_barrel{
            static_cast<float>(gameplay.pos_barrel.x()),
            static_cast<float>(gameplay.pos_barrel.y()),
            static_cast<float>(width_barrel),
            static_cast<float>(height_barrel),
        };
        if (gameplay.is_cool_down) {
            camera.render_texture(resource_mgr.TextureBarrelIdle(), nullptr, &rect_barrel, gameplay.angle_barrel,
                                  &gameplay.center_barrel);
        } else {
            gameplay.animation_barrel_fire.set_rotation(gameplay.angle_barrel);
            gameplay.animation_barrel_fire.on_render(camera);
        }
    }
    // 绘制生命值
    {
        int width;
        int height;
        SDL_QueryTexture(resource_mgr.TextureHeart(), nullptr, nullptr, &width, &height);
        for (int i = 0; i < gameplay.hp; i++) {
            SDL_Rect rect_dst{15 + (width + 10) * i, 15, width, height};
            SDL_RenderCopy(renderer_, resource_mgr.TextureHeart(), nullptr, &rect_dst);
        }
    }
    // 绘制得分
    {
        auto str_score = fmt::format("SCORE: {}", gameplay.score);
        auto suf_score_bg = TTF_RenderUTF8_Blended(resource_mgr.Font(), str_score.c_str(), {55, 55, 55, 255});
        auto suf_score_fg = TTF_RenderUTF8_Blended(resource_mgr.Font(), str_score.c_str(), {255, 255, 255, 255});
        auto tex_score_bg = SDL_CreateTextureFromSurface(renderer_, suf_score_bg);
        auto tex_score_fg = SDL_CreateTextureFromSurface(renderer_, suf_score_fg);
        SDL_Rect rect_score = {1280 - suf_score_bg->w - 15, 15, suf_score_bg->w, suf_score_bg->h};
        SDL_RenderCopy(renderer_, tex_score_bg, nullptr, &rect_score);
        rect_score.x -= 2;
        rect_score.y -= 2;
        SDL_RenderCopy(renderer_, tex_score_fg, nullptr, &rect_score);
        SDL_FreeSurface(suf_score_bg);
        SDL_FreeSurface(suf_score_fg);
        SDL_DestroyTexture(tex_score_bg);
        SDL_DestroyTexture(tex_score_fg);
    }
    // 绘制准星
    {
        int width;
        int height;
        SDL_QueryTexture(resource_mgr.TextureCrosshair(), nullptr, nullptr, &width, &height);
        SDL_FRect rect_crosshair{
            static_cast<float>(gameplay.pos_crosshair.x() - width / 2.0),
            static_cast<float>(gameplay.pos_crosshair.y() - height / 2.0),
            static_cast<float>(width),
            static_cast<float>(height),
        };
        camera.render_texture(resource_mgr.TextureCrosshair(), nullptr, &rect_crosshair, 0, nullptr);
    }
}

}  // namespace sdl2
}  // namespace pyc
