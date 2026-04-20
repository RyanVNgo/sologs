
#include <gtest/gtest.h>

#include <filesystem>

#include "db/database.h"


TEST(DatabaseTests, db_init_file_dne) {
    const char* file_name = "db_init_file_dne.sqlite";

    try {
        SQLiteDatabase db(file_name);
    } catch (std::exception& e) {
        FAIL();
    }

    EXPECT_TRUE(std::filesystem::exists(file_name));
    std::filesystem::remove(file_name);
}

TEST(DatabaseTests, db_init_file_exists) {
    const char* file_name = "db_init_exists.sqlite";

    try {
        SQLiteDatabase db(file_name);
    } catch (std::exception& e) {
        FAIL();
    }

    try {
        SQLiteDatabase db(file_name);
    } catch (std::exception& e) {
        FAIL();
    }

    EXPECT_TRUE(std::filesystem::exists(file_name));
    std::filesystem::remove(file_name);
}

