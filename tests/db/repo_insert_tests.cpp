
#include <filesystem>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/log_repository.h"
#include "domain/log_entry.h"


TEST(RepoTests, repo_insert) {
    const char* db_filename = "repo_init.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    LogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    LogEntry test_entry{
        .message = "test message",
        .level = "INFO",
        .source = "repo_insert test"
    };

    EXPECT_TRUE(repo.insert(test_entry));
    EXPECT_TRUE(util_validate_row_count(db.get(), "logs", 1));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(RepoTests, repo_batch_insert) {
    const char* db_filename = "repo_init.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    LogRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    std::vector<LogEntry> test_entries;
    const size_t entry_count = 3;
    for (size_t i = 0; i < entry_count; i++) {
        test_entries.push_back(
            {
                .message = "test message " + std::to_string(i),
                .level = "INFO",
                .source = "repo_insert test"
            }
        );
    }

    EXPECT_TRUE(repo.insert_batch(test_entries));
    EXPECT_TRUE(util_validate_row_count(db.get(), "logs", entry_count));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

