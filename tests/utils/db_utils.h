
#pragma once

#include <vector>
#include <memory>

#include <sqlite3.h>


using SqliteHandle = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;

SqliteHandle util_open_database(const char* path);

bool util_exec_query(sqlite3* db, const char* query);

bool util_table_exists(sqlite3* db, const char* table_name);
bool util_validate_row_count(sqlite3* db, const char* table_name, int count);
bool util_validate_rows_exist(
        sqlite3* db, 
        const char* table_name, 
        const std::vector<std::vector<std::string>>& rows
);

bool util_insert_log(
        sqlite3* db,
        const std::string& message,
        const std::string& level,
        const std::string& source,
        const std::string& timestamp = ""
);

