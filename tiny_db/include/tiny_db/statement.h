#pragma once

namespace tiny_db {

class Statement {
public:
    enum class Type {
        kInsert,
        kSelect,
    };

    Type type;
};

}  // namespace tiny_db
