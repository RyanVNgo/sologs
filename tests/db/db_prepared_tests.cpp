
#include <filesystem>
#include <format>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/database.h"


TEST(DatabaseTests, db_prepared_valid) {
    const char* db_filename = "db_prepared_valid.sqlite";
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

    const std::vector<std::pair<std::string, Row>> valid_prepares = {
        {
            std::format("INSERT INTO {} (message, level, source) VALUES (?, ?, ?);", table_name),
            {"test log", "INFO", "tests"}
        },
        {
            std::format("INSERT INTO {} (message, level) VALUES (?, ?);", table_name),
            {"test log", "INFO"}
        }
    };
    for (const auto& [query, row] : valid_prepares) {
        ASSERT_TRUE(test_db->execute_prepared(query, row));
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), valid_prepares.size()));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(DatabaseTests, db_prepared_invalid) {
    const char* db_filename = "db_prepared_invalid.sqlite";
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

    const std::vector<std::pair<std::string, Row>> invalid_prepares = {
        {
            std::format("INSERT INTO {} (message, source, level) VALUES (?, ?, ?);", table_name),
            {"test log", "tests"}
        },
        {
            std::format("INSERT INTO {} (message, level, source) VALUES (?, ?);", table_name),
            {"test log", "INFO"}
        },
        {
            std::format("INSERT INTO {} (message, level, source) VALUES (?, ?);", table_name),
            {"test log", "INFO", "tests"}
        },
        {
            std::format("INSERT INTO {} (message, level) VALUES (?, ?, ?);", table_name),
            {"test log", "INFO", "tests"}
        }
    };
    for (const auto& [query, row] : invalid_prepares) {
        ASSERT_FALSE(test_db->execute_prepared(query, row)) << query;
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), 0));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}


