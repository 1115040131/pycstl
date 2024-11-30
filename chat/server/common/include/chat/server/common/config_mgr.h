#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "common/singleton.h"
#include "logger/logger.h"

namespace pyc {
namespace chat {

class ConfigMgr : public Singleton<ConfigMgr> {
    friend class Singleton<ConfigMgr>;

public:
    bool SetSection(const std::string& section);

    std::optional<std::string> GetConfig(const std::string& section, const std::string& key) const;
    std::optional<std::string> GetConfig(const std::string& key) const;

    std::optional<int> GetConfigInt(const std::string& section, const std::string& key) const;
    std::optional<int> GetConfigInt(const std::string& key) const;

    const std::string& GetSection() const { return section_; }

private:
    ConfigMgr();

private:
    using SectionInfo = std::unordered_map<std::string, std::string>;
    std::unordered_map<std::string, SectionInfo> data_;
    std::string section_;
};

inline Logger _g_config_mgr_logger("ConfigMgr");

}  // namespace chat
}  // namespace pyc

#define SET_SECTION(section)                                                          \
    if (!::pyc::chat::ConfigMgr::GetInstance().SetSection(section)) {                 \
        ::pyc::chat::_g_config_mgr_logger.fatal("Config[\"{}\"] not found", section); \
    }

#define GET_SECTION() ::pyc::chat::ConfigMgr::GetInstance().GetSection()

#define GET_CONFIG(var, section, key)                                                                  \
    std::string var;                                                                                   \
    {                                                                                                  \
        auto config = ::pyc::chat::ConfigMgr::GetInstance().GetConfig(section, key);                   \
        if (!config) {                                                                                 \
            ::pyc::chat::_g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found", section, key); \
        }                                                                                              \
        var = *config;                                                                                 \
    }

#define GET_CONFIG_INT(var, section, key)                                                                        \
    int var;                                                                                                     \
    {                                                                                                            \
        auto config = ::pyc::chat::ConfigMgr::GetInstance().GetConfigInt(section, key);                          \
        if (!config) {                                                                                           \
            ::pyc::chat::_g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found or convert fail", section, \
                                                    key);                                                        \
        }                                                                                                        \
        var = *config;                                                                                           \
    }

#define GET_SECTION_CONFIG(var, key)                                                                             \
    std::string var;                                                                                             \
    {                                                                                                            \
        const auto& config_mgr = ::pyc::chat::ConfigMgr::GetInstance();                                          \
        if (config_mgr.GetSection().empty()) {                                                                   \
            ::pyc::chat::_g_config_mgr_logger.fatal("Section not set");                                          \
        }                                                                                                        \
        auto config = config_mgr.GetConfig(key);                                                                 \
        if (!config) {                                                                                           \
            ::pyc::chat::_g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found", config_mgr.GetSection(), \
                                                    key);                                                        \
        }                                                                                                        \
        var = *config;                                                                                           \
    }

#define GET_SECTION_CONFIG_INT(var, key)                                                                \
    int var;                                                                                            \
    {                                                                                                   \
        const auto& config_mgr = ::pyc::chat::ConfigMgr::GetInstance();                                 \
        if (config_mgr.GetSection().empty()) {                                                          \
            ::pyc::chat::_g_config_mgr_logger.fatal("Section not set");                                 \
        }                                                                                               \
        auto config = config_mgr.GetConfigInt(key);                                                     \
        if (!config) {                                                                                  \
            ::pyc::chat::_g_config_mgr_logger.fatal("Config[\"{}\"][\"{}\"] not found or convert fail", \
                                                    config_mgr.GetSection(), key);                      \
        }                                                                                               \
        var = *config;                                                                                  \
    }
