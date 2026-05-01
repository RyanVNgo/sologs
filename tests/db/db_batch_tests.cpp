
#include <format>
#include <filesystem>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/database.h"


TEST(Database, batched_valid) {
    const char* db_filename = "batched_valid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    auto db = util_open_database(db_filename);
    const std::string table_name = "test_table";
    const std::string setup_query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    ASSERT_TRUE(util_exec_query(db.get(), setup_query.c_str()));

    const std::string valid_query = std::format(R"(
        INSERT INTO {} (message, level, source) 
        VALUES (?, ?, ?);
        )", 
        table_name
    );
    const std::vector<Row> valid_data = {
        {"test log 1", "INFO", "tests"},
        {"test log 2", "INFO"},
        {"test log 3", "INFO", "tests"}
    };

    EXPECT_TRUE(test_db->execute_prepared_batched(valid_query, valid_data));

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), valid_data.size()));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Database, batched_invalid_query) {
    const char* db_filename = "batched_invalid_query.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    // Setup table
    auto db = util_open_database(db_filename);
    const std::string table_name = "test_table";
    const std::string setup_query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    ASSERT_TRUE(util_exec_query(db.get(), setup_query.c_str()));

    const std::vector<std::string> invalid_queries = {
        std::format(R"(
            INSERT INTO {} (message, level, source) 
            VALUES (?, ?);)", 
            table_name
        ),
        std::format(R"(
            INSERT INTO {} (message, level) 
            VALUES (?, ?, ?);)", 
            table_name
        ),
    };
    const std::vector<Row> valid_data = {
        {"test log 1", "INFO", "tests"},
        {"test log 2", "INFO"},
        {"test log 3", "INFO", "tests"}
    };

    for (const auto& query : invalid_queries) {
        EXPECT_FALSE(test_db->execute_prepared_batched(query, valid_data));
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), 0));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Database, batched_invalid_data) {
    const char* db_filename = "batched_invalid_data.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    // Setup table
    auto db = util_open_database(db_filename);
    const std::string table_name = "test_table";
    const std::string setup_query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    ASSERT_TRUE(util_exec_query(db.get(), setup_query.c_str()));

    const std::string valid_query = std::format(R"(
        INSERT INTO {} (message, level, source) 
        VALUES (?, ?, ?);
        )", 
        table_name
    );
    const std::vector<std::vector<Row>> invalid_data = {
        {
            {"test log 1", "INFO", "tests"},
            {"test log 2"},
            {"test log 3", "INFO", "tests"}
        },
        {
            {"test log 1", "INFO", "tests"},
            {"test log 3", "INFO", "tests"},
            {"test log 2", "INFO", "tests", "extra"}
        }
    };

    for (const auto& data : invalid_data) {
        EXPECT_FALSE(test_db->execute_prepared_batched(valid_query, data));
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), 0));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

