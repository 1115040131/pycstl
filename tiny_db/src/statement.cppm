export module tiny_db.statement;

export import tiny_db.row;

export namespace tiny_db {

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
