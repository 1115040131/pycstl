#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "common/singleton.h"
#include "logger/logger.h"

namespace pyc {
namespace chat {

struct SectionInfo {
    std::unordered_map<std::string, std::string> data;

    std::optional<std::string> operator[](const std::string& key) const {
        auto iter = data.find(key);
        if (iter == data.end()) {
            return {};
        }
        return iter->second;
    }
};

class ConfigMgr : public Singleton<ConfigMgr> {
    friend class Singleton<ConfigMgr>;

public:
    const SectionInfo& operator[](const std::string& section) { return data_[section]; }

private:
    ConfigMgr();

private:
    std::unordered_map<std::string, SectionInfo> data_;
};

inline Logger _g_config_mgr_logger("ConfigMgr");

}  // namespace chat
}  // namespace pyc

#define GET_CONFIG(var, section, key)                                                        \
    std::string var;                                                                         \
    {                                                                                        \
        auto config = ::pyc::chat::ConfigMgr::GetInstance()[section][key];                   \
        if (!config) {                                                                       \
            ::pyc::chat::_g_config_mgr_logger.fatal("Config " section " " key " not found"); \
        }                                                                                    \
        var = *config;                                                                       \
    }
