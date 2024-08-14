#pragma once

#include <common/singleton.h>

#include <string>
#include <unordered_map>

namespace pyc {
namespace chat {

struct SectionInfo {
    std::unordered_map<std::string, std::string> data;

    const std::string& operator[](const std::string& key) const {
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
