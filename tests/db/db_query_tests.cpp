
#include <filesystem>
#include <format>

#include <gtest/gtest.h>

#include <db_utils.h>

#include <database/database.h>


TEST(Database, query_valid) {
    const char* db_filename = "query_valid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    auto db = util_open_database(db_filename);
    const std::string table_name = "test_table";
    const std::string setup_table_query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    const std::vector<Row> setup_data = {
        {"test message 1", "INFO", "query tests"},
        {"test message 2", "WARN", "query tests"},
        {"test message 3", "ERROR", "query tests"}
    };
    std::vector<std::string> setup_data_queries;
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO {} (message, level, source)
            VALUES ('{}', '{}', '{}');
            )", 
            table_name,
            data[0], data[1], data[2]
        );
        setup_data_queries.push_back(query);
    }

    ASSERT_TRUE(util_exec_query(db.get(), setup_table_query.c_str()));
    for (const auto& query : setup_data_queries) {
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }
    ASSERT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), setup_data_queries.size()));
    
    {
    const std::string test_query = std::format(R"(
        SELECT id, message, level, source, timestamp FROM {};
        )", 
        table_name
    );
    QueryResult res = test_db->query(test_query);
    EXPECT_EQ(res.size(), setup_data_queries.size());
    }

    {
    const std::string test_query = std::format(R"(
        SELECT id, message, level, source, timestamp FROM {}
        WHERE level='INFO';
        )", 
        table_name
    );
    QueryResult res = test_db->query(test_query);
    EXPECT_EQ(res.size(), 1);
    }

    {
    const std::string test_query = std::format(R"(
        SELECT id, message, level, source, timestamp FROM {}
        WHERE NOT level='INFO';
        )", 
        table_name
    );
    QueryResult res = test_db->query(test_query);
    EXPECT_EQ(res.size(), 2);
    }

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Database, query_invalid) {
    const char* db_filename = "query_invalid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    auto db = util_open_database(db_filename);
    const std::string table_name = "test_table";
    const std::string setup_table_query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    const std::vector<Row> setup_data = {
        {"test message 1", "INFO", "query tests"},
        {"test message 2", "WARN", "query tests"},
        {"test message 3", "ERROR", "query tests"}
    };
    std::vector<std::string> setup_data_queries;
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO {} (message, level, source)
            VALUES ('{}', '{}', '{}');
            )", 
            table_name,
            data[0], data[1], data[2]
        );
        setup_data_queries.push_back(query);
    }

    ASSERT_TRUE(util_exec_query(db.get(), setup_table_query.c_str()));
    for (const auto& query : setup_data_queries) {
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }
    ASSERT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), setup_data_queries.size()));
    
    { // Malformed query
    const std::string test_query = std::format(R"(
        SELECT id, message, level source timestamp FROM {};
        )", 
        table_name
    );
    EXPECT_THROW(test_db->query(test_query), std::exception);
    }

    { // Empty query
    const std::string test_query = "";
    EXPECT_THROW(test_db->query(test_query), std::exception);
    }

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

