#include "chat/server/common/config_mgr.h"

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
            section_info.data[key] = value.get_value<std::string>();
        }
        data_.emplace(section_name, std::move(section_info));
    }

    fmt::println("========== Server Config ==========");
    for (const auto& [section_name, section_info] : data_) {
        fmt::println("[{}]", section_name);
        for (const auto& [key, value] : section_info.data) {
            fmt::println("{}={}", key, value);
        }
    }
    fmt::println("===================================");
}

}  // namespace chat
}  // namespace pyc
