module;

#include <charconv>
#include <filesystem>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "logger/logger.h"

module chat.server.common.config_mgr;

namespace pyc {
namespace chat {

static Logger _g_config_mgr_logger("ConfigMgr");

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

void SetSectionOrDie(std::string_view section) {
    if (!ConfigMgr::GetInstance().SetSection(std::string(section))) {
        _g_config_mgr_logger.fatal("Config[\"{}\"] not found", section);
    }
}

const std::string& CurrentSection() { return ConfigMgr::GetInstance().GetSection(); }

std::string GetConfigOrDie(std::string_view section, std::string_view key) {
    auto config = ConfigMgr::GetInstance().GetConfig(std::string(section), std::string(key));
    if (!config) {
        _g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found", section, key);
    }
    return *config;
}

int GetConfigIntOrDie(std::string_view section, std::string_view key) {
    auto config = ConfigMgr::GetInstance().GetConfigInt(std::string(section), std::string(key));
    if (!config) {
        _g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found or convert fail", section, key);
    }
    return *config;
}

std::string GetSectionConfigOrDie(std::string_view key) {
    const auto& config_mgr = ConfigMgr::GetInstance();
    if (config_mgr.GetSection().empty()) {
        _g_config_mgr_logger.fatal("Section not set");
    }
    auto config = config_mgr.GetConfig(std::string(key));
    if (!config) {
        _g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found", config_mgr.GetSection(), key);
    }
    return *config;
}

int GetSectionConfigIntOrDie(std::string_view key) {
    const auto& config_mgr = ConfigMgr::GetInstance();
    if (config_mgr.GetSection().empty()) {
        _g_config_mgr_logger.fatal("Section not set");
    }
    auto config = config_mgr.GetConfigInt(std::string(key));
    if (!config) {
        _g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found or convert fail", config_mgr.GetSection(),
                                   key);
    }
    return *config;
}

}  // namespace chat
}  // namespace pyc
