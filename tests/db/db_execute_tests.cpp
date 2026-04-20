
#include <format>

#include <gtest/gtest.h>

#include "sqlite3.h"

#include "db/database.h"


TEST(DatabaseTests, db_execute_valid) {
    const char* db_filename = "db_execute_valid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    try {
        test_db = std::make_unique<SQLiteDatabase>(db_filename);
    } catch (...) {
        FAIL();
    }
    
    std::string table_name = "test_table";
    std::string query = std::format(
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

    struct Tables {
        std::vector<std::string> names;
    };
    Tables tables;

    auto callback = [](void* out, int col_count, char** col_vals, char**) -> int {
        Tables* t = (Tables*)out;
        for (size_t i = 0; i < col_count; i++) {
            t->names.push_back(col_vals[i]);
        }
        return 0;
    };

    sqlite3* db;
    sqlite3_open(db_filename, &db);
    sqlite3_exec(
            db,
            "SELECT name FROm sqlite_schema WHERE type='table' AND name NOT LIKE 'sqlite_%';",
            callback,
            &tables,
            nullptr
    );

    EXPECT_GE(tables.names.size(), 1);
    auto target_iter = std::find(
            tables.names.begin(),
            tables.names.end(),
            table_name
    );
    EXPECT_NE(target_iter, tables.names.end());
}

TEST(DatabaseTests, db_execute_invalid) {
    const char* db_filename = "db_execute_invalid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    try {
        test_db = std::make_unique<SQLiteDatabase>(db_filename);
    } catch (...) {
        FAIL();
    }
    
    std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS test_table ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
    );

    const std::vector<std::string> invalid_queries = {
        "SELEC * FROM test_table;",
        "SELECT * FROM ;",
        "SELECT * FROM test_table WHERE message = 'Hello;",
        "SELECT id, message, FROM test_table;",
        "SELECT * FROM test_table WHERE;",
        "SELECT (id, message FROM test_table;",
        "INSERT INTO test_table id, message VALUES (1, 'Test');",
        "INSERT INTO test_table (id, message) VALUES (1, 'Test', 'Extra');",
        "UPDATE test_table SET WHERE id = 1;",
        "SELECT id test_table;",
        "SELECT * FROM test_table JOIN ON test_table.id = test_table.id;",
    };

    for (const auto& query : invalid_queries) {
        EXPECT_FALSE(test_db->execute(query)) << query;
    }
}

