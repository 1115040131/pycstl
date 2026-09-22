module;

#include <cstdint>

export module tiny_db.defines;

export namespace tiny_db {

inline constexpr uint32_t kPageSize = 4096;
inline constexpr uint32_t kTableMaxPages = 100;

}  // namespace tiny_db
