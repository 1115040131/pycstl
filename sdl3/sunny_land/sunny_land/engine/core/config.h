#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <nlohmann/json_fwd.hpp>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

// --- 默认配置值 ---
struct ConfigDetail {
    // 窗口设置
    struct Window {
        std::string title = "SunnyLand";
        int width = 1280;
        int height = 720;
        bool resizable = true;
    } window;

    // 图形设置
    struct Graphics {
        bool vsync = true;  ///< @brief 是否启用垂直同步
    } graphics;

    // 性能设置
    struct Performance {
        int target_fps = 144;  ///< @brief 目标 FPS 设置，0 表示不限制
    } performance;

    // 音频设置
    struct Audio {
        float music_volume = 0.5f;
        float sound_volume = 0.5f;
    } audio;

    // 存储动作名称到 SDL Scancode 名称列表的映射
    std::unordered_map<std::string, std::vector<std::string>> input_mappings = {
        // 提供一些合理的默认值，以防配置文件加载失败或缺少此部分
        {"move_left", {"A", "Left"}}, {"move_right", {"D", "Right"}}, {"move_up", {"W", "Up"}},
        {"move_down", {"S", "Down"}}, {"jump", {"J", "Space"}},       {"attack", {"K", "MouseLeft"}},
        {"pause", {"P", "Escape"}},
        // 可以继续添加更多默认动作
    };
};

/**
 * @brief 管理应用程序的配置设置。
 *
 * 提供配置项的默认值，并支持从 JSON 文件加载/保存配置。
 * 如果加载失败或文件不存在，将使用默认值。
 */
class Config final : Noncopyable {
public:
    explicit Config(std::string_view filepath);  ///< @brief 构造函数，指定配置文件路径。

    ///< @brief 从指定的 JSON 文件加载配置。成功返回 true，否则返回 false。
    bool loadFromFile(std::string_view filepath);
    ///< @brief 将当前配置保存到指定的 JSON 文件。成功返回 true，否则返回 false。
    [[nodiscard]] bool saveToFile(std::string_view filepath);

    const ConfigDetail& getConfigDetail() const { return config_detail; }

private:
    ConfigDetail config_detail{};
};

}  // namespace pyc::sunny_land
