
#include <filesystem>
#include <format>

#include <gtest/gtest.h>

#include "db_utils.h"

#include <database/log_repository.h>


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

    std::vector<LogEntry> result = repo.get_all({});
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

    std::vector<LogEntry> result = repo.get_all({});
    EXPECT_EQ(result.size(), 0);

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_with_level_filter) {
    const char* db_filename = "get_all_level_filter.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    const std::vector<Row> setup_data = {
        {"test message 1", "INFO", "filter tests"},
        {"test message 2", "WARN", "filter tests"},
        {"test message 3", "ERROR", "filter tests"}
    };
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('{}', '{}', '{}');
            )",
            data[0], data[1], data[2]
        );
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }

    std::vector<LogEntry> result = repo.get_all({.level = "ERROR"});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].message, "test message 3");
    EXPECT_EQ(result[0].level, "ERROR");

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_with_source_filter) {
    const char* db_filename = "get_all_source_filter.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    const std::vector<Row> setup_data = {
        {"test message 1", "INFO", "source-a"},
        {"test message 2", "INFO", "source-b"},
        {"test message 3", "INFO", "source-a"}
    };
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('{}', '{}', '{}');
            )",
            data[0], data[1], data[2]
        );
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }

    std::vector<LogEntry> result = repo.get_all({.source = "source-a"});
    EXPECT_EQ(result.size(), 2);
    for (const auto& log : result) {
        EXPECT_EQ(log.source, "source-a");
    }

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_with_limit) {
    const char* db_filename = "get_all_limit.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    for (int i = 0; i < 5; i++) {
        std::string query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('test message {}', 'INFO', 'limit tests');
            )", i
        );
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }

    std::vector<LogEntry> result = repo.get_all({.limit = 2});
    EXPECT_EQ(result.size(), 2);

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_with_time_range) {
    const char* db_filename = "get_all_time_range.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    ASSERT_TRUE(util_insert_log(db.get(), "old message", "INFO", "time tests", "2024-01-01 00:00:00"));
    ASSERT_TRUE(util_insert_log(db.get(), "middle message", "INFO", "time tests", "2024-06-15 12:00:00"));
    ASSERT_TRUE(util_insert_log(db.get(), "new message", "INFO", "time tests", "2024-12-31 23:59:59"));

    std::vector<LogEntry> result = repo.get_all({
        .since = "2024-01-01 00:00:00",
        .until = "2024-06-30 23:59:59"
    });

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].message, "middle message");
    EXPECT_EQ(result[1].message, "old message");

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(Repo, get_all_with_combined_filters) {
    const char* db_filename = "get_all_combined.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlLogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    const std::vector<Row> setup_data = {
        {"message 1", "INFO", "source-x"},
        {"message 2", "ERROR", "source-x"},
        {"message 3", "INFO", "source-y"},
        {"message 4", "ERROR", "source-y"},
        {"message 5", "WARN", "source-x"}
    };
    for (const auto& data : setup_data) {
        std::string query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('{}', '{}', '{}');
            )",
            data[0], data[1], data[2]
        );
        ASSERT_TRUE(util_exec_query(db.get(), query.c_str()));
    }

    std::vector<LogEntry> result = repo.get_all({.level = "ERROR", .source = "source-x"});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].message, "message 2");
    EXPECT_EQ(result[0].level, "ERROR");
    EXPECT_EQ(result[0].source, "source-x");

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

