#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace tiny_db {

inline constexpr uint32_t kUsernameSize = 31;
inline constexpr uint32_t kEmailSize = 255;

struct Row {
    uint32_t id = 0;
    char username[kUsernameSize + 1] = {0};
    char email[kEmailSize + 1] = {0};

    static std::string Serialize(const Row& row);
    static Row Deserialize(std::string_view data);

    std::string ToString() const;
};

inline constexpr uint32_t kRowSize = sizeof(Row);

}  // namespace tiny_db
