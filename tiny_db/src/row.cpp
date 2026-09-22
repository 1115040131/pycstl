module;

#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

#include <fmt/format.h>

module tiny_db.row;

namespace tiny_db {

std::string Row::Serialize(const Row& row) {
    std::string buffer;
    buffer.resize(kRowSize);
    std::memcpy(buffer.data(), &row, kRowSize);
    return buffer;
}

Row Row::Deserialize(std::string_view data) {
    if (data.size() != kRowSize) {
        throw std::invalid_argument("Data size does not match row size.");
    }

    Row row;
    std::memcpy(&row, data.data(), kRowSize);
    return row;
}

std::string Row::ToString() const { return fmt::format("({}, {}, {})", id, username, email); }

}  // namespace tiny_db
