
#include <filesystem>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/log_repository.h"


TEST(Repo, init) {
    const char* db_filename = "init.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));

    auto db = util_open_database(db_filename);
    SqlLogRepository repo(*test_db.get());

    EXPECT_TRUE(util_table_exists(db.get(), "logs"));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

