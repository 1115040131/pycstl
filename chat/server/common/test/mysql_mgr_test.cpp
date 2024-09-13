#include <gtest/gtest.h>
#include <mysqlx/xdevapi.h>

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/mysql_mgr.h"

namespace pyc {
namespace chat {

TEST(MysqlMgrTest, ConnectionTest) {
    GET_CONFIG(host, "Mysql", "Host");
    GET_CONFIG_INT(port, "Mysql", "Port");
    GET_CONFIG(user, "Mysql", "User");
    GET_CONFIG(password, "Mysql", "Password");

    auto session = mysqlx::Session(host, port, user, password);

    mysqlx::RowResult res = session.sql("show variables like 'version'").execute();
    std::stringstream version;
    version << res.fetchOne().get(1).get<std::string>();
    int major_version;
    version >> major_version;
    EXPECT_TRUE(major_version >= 8);

    const std::string kSchema = "test_schema";
    const std::string kTable = "test_table";

    if (session.getSchema(kSchema).existsInDatabase()) {
        session.dropSchema(kSchema);
    }

    EXPECT_FALSE(session.getSchema(kSchema).existsInDatabase());
    auto schema = session.createSchema(kSchema, true);
    EXPECT_TRUE(schema.existsInDatabase());

    session.sql(fmt::format("USE {}", kSchema)).execute();
    session
        .sql(fmt::format("CREATE TABLE {} ({});", kTable, R"(
        id INT NOT NULL AUTO_INCREMENT,
        value1 VARCHAR(50),
        value2 VARCHAR(50),
        PRIMARY KEY (id)
    )"))
        .execute();
    auto table = schema.getTable(kTable);
    EXPECT_TRUE(table.existsInDatabase());

    // 使用 SQL 语句创建一个存储过程
    session
        .sql(R"(
            CREATE FUNCTION hello_world()
            RETURNS VARCHAR(50)
            DETERMINISTIC
            BEGIN
                RETURN 'Hello, World!';
            END
        )")
        .execute();

    {
        auto result = session.sql("SELECT hello_world()").execute();
        for (const auto& row : result) {
            EXPECT_EQ(row.colCount(), 1);
            EXPECT_EQ(row[0].getType(), mysqlx::Value::Type::STRING);
            EXPECT_EQ(row[0].get<std::string>(), "Hello, World!");
        }
    }
    {
        EXPECT_EQ(table.insert("value1", "value2").values("1_1", "1_2").execute().getAutoIncrementValue(), 1);
        EXPECT_EQ(table.insert("value1", "value2").values("2_1", "2_2").execute().getAutoIncrementValue(), 2);
    }
    {
        mysqlx::RowResult result = table.select("*").execute();
        for (const auto& row : result) {
            for (unsigned int i = 0; i < row.colCount(); ++i) {
                std::cout << row[i] << '\t';
            }
            std::cout << '\n';
        }
    }

    session.dropSchema(kSchema);
}

TEST(MysqlMgrTest, RegUser) {
    auto& mysql_mgr = MysqlMgr::GetInstance();

    EXPECT_EQ(mysql_mgr.RegUser("test1", "test1@test.com", "test1"), 1);
    EXPECT_EQ(mysql_mgr.RegUser("test1", "test1@test.com", "test1"), 0);
    EXPECT_EQ(mysql_mgr.RegUser("test2", "test1@test.com", "test2"), 0);
    EXPECT_EQ(mysql_mgr.RegUser("test2", "test2@test.com", "test2"), 2);
}

}  // namespace chat
}  // namespace pyc