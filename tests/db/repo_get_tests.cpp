
#include <filesystem>
#include <format>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/log_repository.h"


TEST(Repo, get_all) {
    const char* db_filename = "get_all.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    const std::vector<Row> setup_data = {
        {"test message 1", "INFO", "query tests"},
        {"test message 2", "WARN", "query tests"},
        {"test message 3", "ERROR", "query tests"}
    };
    std::vector<std::string> setup_data_queries;
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('{}', '{}', '{}');
            )", 
            data[0], data[1], data[2]
        );
        setup_data_queries.push_back(query);
    }
    for (const auto& query : setup_data_queries) {
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }
    ASSERT_TRUE(util_validate_row_count(db.get(), "logs", setup_data_queries.size()));

    std::vector<LogEntry> result = repo.get_all();
    EXPECT_EQ(result.size(), setup_data_queries.size());

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_empty) {
    const char* db_filename = "get_all_empty.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    std::vector<LogEntry> result = repo.get_all();
    EXPECT_EQ(result.size(), 0);

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

