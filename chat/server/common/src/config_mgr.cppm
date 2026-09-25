module;

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "common/singleton.h"

export module chat.server.common.config_mgr;

export namespace pyc::chat {

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

// 读配置的便捷函数：读不到（或无法转换成 int）即 fatal。
void SetSectionOrDie(std::string_view section);

const std::string& CurrentSection();

std::string GetConfigOrDie(std::string_view section, std::string_view key);

int GetConfigIntOrDie(std::string_view section, std::string_view key);

std::string GetSectionConfigOrDie(std::string_view key);

int GetSectionConfigIntOrDie(std::string_view key);

}  // namespace pyc::chat
