#include "sunny_land/engine/core/config.h"

#include <fstream>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ConfigDetail::Window, title, width, height, resizable);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ConfigDetail::Graphics, vsync);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ConfigDetail::Performance, target_fps);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ConfigDetail::Audio, music_volume, sound_volume);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ConfigDetail, window, graphics, performance, audio,
                                                input_mappings);

Config::Config(std::string_view filepath) { loadFromFile(filepath); }

bool Config::loadFromFile(std::string_view filepath) {
    std::ifstream file(filepath.data());
    if (!file.is_open()) {
        spdlog::warn("配置文件 '{}' 未找到。使用默认设置并创建默认配置文件。", filepath);
        if (!saveToFile(filepath)) {
            spdlog::error("无法创建默认配置文件 '{}'。", filepath);
            return false;
        }
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        config_detail = j.get<ConfigDetail>();
        nlohmann::ordered_json j2 = config_detail;
        spdlog::info("成功从 '{}' 加载配置:\n{}", filepath, j2.dump(4));
        return true;
    } catch (const std::exception& e) {
        spdlog::error("读取配置文件 '{}' 时出错：{}。使用默认设置。", filepath, e.what());
    }
    return false;
}

bool Config::saveToFile(std::string_view filepath) {
    std::ofstream file(filepath.data());
    if (!file.is_open()) {
        spdlog::error("无法打开配置文件 '{}' 进行写入。", filepath);
        return false;
    }

    try {
        nlohmann::ordered_json j = config_detail;
        file << j.dump(4);
        spdlog::info("成功将配置保存到 '{}'。", filepath);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("写入配置文件 '{}' 时出错：{}", filepath, e.what());
    }
    return false;
}

}  // namespace pyc::sunny_land