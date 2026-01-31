#pragma once

#include <chrono>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

using namespace std::chrono_literals;

/**
 * @brief 单一波次数据
 * @note 包含下一波次间隔、本波次敌人生成间隔和“本波次敌人类型-数量”对
 */
struct Wave {
    std::chrono::duration<float> next_wave_interval_{};       ///< @brief 下一波次间隔
    std::chrono::duration<float> spawn_interval_{};           ///< @brief 本波次敌人生成间隔
    std::vector<std::pair<entt::id_type, int>> enemy_types_;  ///< @brief 敌人类型-数量对
};

/**
 * @brief 多波次数据，即一关中所有的波次
 * @note 包含下一波次倒计时、波次队列
 */
struct Waves {
    std::chrono::duration<float> next_wave_count_down_{};  ///< @brief 下一波次倒计时
    std::queue<Wave> waves_;                               ///< @brief 波次队列
};

/**
 * @brief 关卡数据，包含一关中的波次数据及其他必要信息
 * @note 关卡号、敌人等级、敌人稀有度、关卡名称、地图路径、准备时间、总敌人数量
 */
struct LevelData {
    int level_number_{1};                         ///< @brief 关卡号
    int enemy_level_{1};                          ///< @brief 敌人等级（本关所有敌人统一等级）
    int enemy_rarity_{1};                         ///< @brief 敌人稀有度（本关所有敌人统一稀有度）
    std::string name_;                            ///< @brief 关卡名称
    std::string map_path_;                        ///< @brief 地图路径
    std::chrono::duration<float> prep_time_{5s};  ///< @brief 开局准备时间
    int total_enemy_count_{0};                    ///< @brief 总敌人数量
    Waves waves_data_;                            ///< @brief 波次数据
};

}  // namespace pyc::monster_war