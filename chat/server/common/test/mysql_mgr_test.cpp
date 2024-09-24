#include <fmt/chrono.h>
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

    // 根据当前时间戳生成用户和邮箱用于测试
    auto user1 = fmt::format("pycstl_{}", std::chrono::system_clock::now());
    auto email1 = fmt::format("{}@test.com", user1);
    auto user2 = fmt::format("pycstl_{}", std::chrono::system_clock::now());
    auto email2 = fmt::format("{}@test.com", user2);

    EXPECT_TRUE(mysql_mgr.RegUser(user1, email1, "123") > 0);
    EXPECT_EQ(mysql_mgr.RegUser(user1, email1, "123"), 0);
    EXPECT_EQ(mysql_mgr.RegUser(user2, email1, "123"), 0);
    EXPECT_TRUE(mysql_mgr.RegUser(user2, email2, "123") > 0);

    // TODO: 删除测试数据
}

TEST(MysqlMgrTest, CheckEmail) {
    auto& mysql_mgr = MysqlMgr::GetInstance();

    // 根据当前时间戳生成用户和邮箱用于测试
    auto user1 = fmt::format("pycstl_{}", std::chrono::system_clock::now());
    auto email1 = fmt::format("{}@test.com", user1);
    auto user2 = fmt::format("pycstl_{}", std::chrono::system_clock::now());
    auto email2 = fmt::format("{}@test.com", user2);

    EXPECT_TRUE(mysql_mgr.RegUser(user1, email1, "123") > 0);
    EXPECT_TRUE(mysql_mgr.RegUser(user2, email2, "123") > 0);

    EXPECT_TRUE(*mysql_mgr.CheckEmail(user1, email1));
    EXPECT_FALSE(*mysql_mgr.CheckEmail(user1, email2));
    EXPECT_TRUE(*mysql_mgr.CheckEmail(user2, email2));
    EXPECT_FALSE(*mysql_mgr.CheckEmail(user2, email1));

    // TODO: 删除测试数据
}

TEST(MysqlMgrTest, UpdatePassword) {
    auto& mysql_mgr = MysqlMgr::GetInstance();

    // 根据当前时间戳生成用户和邮箱用于测试
    auto user1 = fmt::format("pycstl_{}", std::chrono::system_clock::now());
    auto email1 = fmt::format("{}@test.com", user1);
    auto password1 = "123";
    auto password2 = "456";

    EXPECT_TRUE(mysql_mgr.RegUser(user1, email1, password1) > 0);

    // 密码相同, 更新失败
    EXPECT_FALSE(mysql_mgr.UpdatePassword(user1, password1).value());
    // 密码不同, 更新成功
    EXPECT_TRUE(mysql_mgr.UpdatePassword(user1, password2).value());

    // TODO: 删除测试数据
}

}  // namespace chat
}  // namespace pyc