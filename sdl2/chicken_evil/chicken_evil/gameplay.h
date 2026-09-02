#pragma once

#include <vector>

#include <Eigen/Core>

#include "chicken_evil/bullet.h"
#include "chicken_evil/chicken.h"
#include "sdl2/common/timer.h"

namespace pyc {
namespace sdl2 {

struct Gameplay {
    bool is_quit = false;

    int hp = 10;                                    // 生命值
    int score = 0;                                  // 得分
    std::vector<Bullet> bullets;                    // 子弹
    std::vector<std::unique_ptr<Chicken>> chicken;  // 鸡

    int num_per_gen = 2;               // 每次生成数量
    Timer timer_generate;              // 生成定时器
    Timer timer_increase_num_per_gen;  // 增加每次生成数量

    Eigen::Vector2d pos_crosshair;                   // 准星位置
    double angle_barrel = 0;                         // 炮管旋转角度
    const Eigen::Vector2d pos_battery = {640, 600};  // 炮台基座中心点
    const Eigen::Vector2d pos_barrel = {592, 585};   // 炮管无旋转默认位置
    const SDL_FPoint center_barrel = {48, 25};       // 炮管旋转中心点

    bool is_cool_down = true;         // 是否冷却时间
    bool is_fire_key_down = false;    // 是否按下开火
    Animation animation_barrel_fire;  // 炮管开火动画
};

}  // namespace sdl2
}  // namespace pyc
