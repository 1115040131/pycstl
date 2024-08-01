#include "tiny_db/row.h"

#include <cstring>
#include <stdexcept>

#include <fmt/format.h>

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
