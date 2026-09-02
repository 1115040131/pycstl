#pragma once

#include "tiny_db/row.h"

namespace tiny_db {

class Statement {
public:
    enum class Type {
        kInsert,
        kSelect,
    };

    Type type;
    Row row_to_insert;
};

}  // namespace tiny_db
