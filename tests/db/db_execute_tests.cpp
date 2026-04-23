
#include <format>
#include <filesystem>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/database.h"


TEST(DatabaseTests, db_execute_valid) {
    const char* db_filename = "db_execute_valid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    EXPECT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
    // Test initial valid query
    const std::string table_name = "test_table";
    const std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        table_name
    );
    EXPECT_TRUE(test_db->execute(query));

    auto db = util_open_database(db_filename);
    ASSERT_TRUE(util_table_exists(db.get(), table_name.c_str()));

    // Test other valid queries
    const std::vector<std::string> valid_queries = {
        std::format("SELECT * FROM {};", table_name),
        std::format("SELECT COUNT(*) FROM {};", table_name),
        std::format("SELECT source, message, timestamp FROM {} WHERE level='INFO';", table_name),
        std::format("SELECT source, message FROM {} WHERE (timestamp BETWEEN '2026-04-01' AND '2026-04-08') AND level='WARNING';", table_name),
        std::format("INSERT INTO {} (message, level, source) VALUES ('test log', 'INFO', 'tests');", table_name),
        std::format("INSERT INTO {} (message, level) VALUES ('test log', 'INFO');", table_name)
    };
    for (const auto& query : valid_queries) {
        EXPECT_TRUE(test_db->execute(query)) << query;
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), table_name.c_str(), 2));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(DatabaseTests, db_execute_invalid) {
    const char* db_filename = "db_execute_invalid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    EXPECT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    
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

    // Test invalid queries
    const std::vector<std::string> invalid_queries = {
        std::format("SELEC * FROM {};", table_name),
        std::format("SELECT COUNT(* FROM {};", table_name),
        std::format("SELECT source message timestamp FROM {} WHERE level='INFO';", table_name),
        std::format("SELECT source, message FROM {} WHERE (timestamp BETWEEN '2026-04-01' AND '2026-04-08') AND level='WARNING';", table_name + "e"),
        std::format("INSERT INTO {} (message, level, source) VALUES ('test log', 'tests');", table_name),
        std::format("INSERT INTO {} (level, source) VALUES ('INFO', 'tests');", table_name)
    };
    for (const auto& query : invalid_queries) {
        EXPECT_FALSE(test_db->execute(query)) << query;
    }

    EXPECT_TRUE(util_validate_row_count(db.get(), "test_table", 0));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

