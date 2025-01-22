#pragma once

#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "common/singleton.h"
#include "sdl2/common/atlas.h"

namespace pyc {
namespace sdl2 {

class ResourceMgr : public Singleton<ResourceMgr> {
    friend class Singleton<ResourceMgr>;
    friend class Engine;

private:
    ResourceMgr() = default;

public:
    SDL_Texture* TextureHeart() const { return tex_heart_; }
    SDL_Texture* TextureBullet() const { return tex_bullet_; }
    SDL_Texture* TextureBattery() const { return tex_battery_; }
    SDL_Texture* TextureCrosshair() const { return tex_crosshair_; }
    SDL_Texture* TextureBackground() const { return tex_background_; }
    SDL_Texture* TextureBarrelIdle() const { return tex_barrel_idle_; }

    const Atlas& AtlasBarrelFire() const { return atlas_barrel_fire_; }
    const Atlas& AtlasChickenFast() const { return atlas_chicken_fast_; }
    const Atlas& AtlasChickenMedium() const { return atlas_chicken_medium_; }
    const Atlas& AtlasChickenSlow() const { return atlas_chicken_slow_; }
    const Atlas& AtlasExplosion() const { return atlas_explosion_; }

    Mix_Music* MusicBgm() const { return music_bgm_; }
    Mix_Music* MusicLoss() const { return music_loss_; }

    Mix_Chunk* SoundHurt() const { return sound_hurt_; }
    Mix_Chunk* SoundFire1() const { return sound_fire_1_; }
    Mix_Chunk* SoundFire2() const { return sound_fire_2_; }
    Mix_Chunk* SoundFire3() const { return sound_fire_3_; }
    Mix_Chunk* SoundExplosion() const { return sound_explosion_; }

    TTF_Font* Font() const { return font_; }

private:
    SDL_Texture* tex_heart_ = nullptr;        // 生命值图标
    SDL_Texture* tex_bullet_ = nullptr;       // 子弹
    SDL_Texture* tex_battery_ = nullptr;      // 炮台基座
    SDL_Texture* tex_crosshair_ = nullptr;    // 光标准星
    SDL_Texture* tex_background_ = nullptr;   // 背景图
    SDL_Texture* tex_barrel_idle_ = nullptr;  // 炮管默认状态

    Atlas atlas_barrel_fire_;     // 炮管开火
    Atlas atlas_chicken_fast_;    // 快速鸡
    Atlas atlas_chicken_medium_;  // 中速鸡
    Atlas atlas_chicken_slow_;    // 慢速鸡
    Atlas atlas_explosion_;       // 爆炸

    Mix_Music* music_bgm_ = nullptr;   // 背景音乐
    Mix_Music* music_loss_ = nullptr;  // 失败音乐

    Mix_Chunk* sound_hurt_;       // 受伤音效
    Mix_Chunk* sound_fire_1_;     // 开火音效 1
    Mix_Chunk* sound_fire_2_;     // 开火音效 2
    Mix_Chunk* sound_fire_3_;     // 开火音效 3
    Mix_Chunk* sound_explosion_;  // 爆炸音效

    TTF_Font* font_ = nullptr;
};

}  // namespace sdl2
}  // namespace pyc
