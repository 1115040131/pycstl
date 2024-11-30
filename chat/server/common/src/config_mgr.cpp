#include "chat/server/common/config_mgr.h"

#include <charconv>
#include <filesystem>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pyc {
namespace chat {

ConfigMgr::ConfigMgr() {
    std::filesystem::path config_path = "chat/server/common/config/config.ini";
    if (!std::filesystem::exists(config_path)) {
        _g_config_mgr_logger.fatal("{} not exists!", config_path.string());
        return;
    }

    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);

    for (const auto& [section_name, section_tree] : pt) {
        SectionInfo section_info;
        for (const auto& [key, value] : section_tree) {
            section_info[key] = value.get_value<std::string>();
        }
        data_.emplace(section_name, std::move(section_info));
    }

    fmt::println("========== Server Config ==========");
    for (const auto& [section_name, section_info] : data_) {
        fmt::println("[{}]", section_name);
        for (const auto& [key, value] : section_info) {
            fmt::println("  {} = {}", key, value);
        }
    }
    fmt::println("===================================");
}

bool ConfigMgr::SetSection(const std::string& section) {
    auto iter = data_.find(section);
    if (iter == data_.end()) {
        return false;
    }
    section_ = section;
    _g_config_mgr_logger.debug("Section \"{}\" set", section);
    return true;
}

std::optional<std::string> ConfigMgr::GetConfig(const std::string& section, const std::string& key) const {
    auto iter = data_.find(section);
    if (iter == data_.end()) {
        return {};
    }
    auto config = iter->second.find(key);
    if (config == iter->second.end()) {
        return {};
    }
    return config->second;
}

std::optional<std::string> ConfigMgr::GetConfig(const std::string& key) const { return GetConfig(section_, key); }

std::optional<int> ConfigMgr::GetConfigInt(const std::string& section, const std::string& key) const {
    auto config = GetConfig(section, key);
    if (!config) {
        return {};
    }
    int value{};
    auto result = std::from_chars(config->data(), config->data() + config->size(), value);
    if (result.ec != std::errc()) {
        return {};
    }
    return value;
}

std::optional<int> ConfigMgr::GetConfigInt(const std::string& key) const { return GetConfigInt(section_, key); }

}  // namespace chat
}  // namespace pyc
