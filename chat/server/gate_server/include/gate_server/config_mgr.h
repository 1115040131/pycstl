#pragma once

#include <common/singleton.h>

#include <optional>
#include <string>
#include <unordered_map>

#include "gate_server/define.h"

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
    const SectionInfo& operator[](const std::string& section) const {
        auto iter = data_.find(section);
        if (iter == data_.end()) {
            return {};
        }
        return iter->second;
    }

private:
    ConfigMgr();

private:
    std::unordered_map<std::string, SectionInfo> data_;
};

}  // namespace chat
}  // namespace pyc

#define GET_CONFIG(var, section, key)                                      \
    std::string var;                                                       \
    {                                                                      \
        auto config = ::pyc::chat::ConfigMgr::GetInstance()[section][key]; \
        if (!config) {                                                     \
            PYC_LOG_FATAL("Config " section " " key " not found");         \
        }                                                                  \
        var = *config;                                                     \
    }
