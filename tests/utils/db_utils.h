
#pragma once

#include <memory>

#include <sqlite3.h>


using SqliteHandle = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;

SqliteHandle util_open_database(const char* path);

bool util_exec_query(sqlite3* db, const char* query);

bool util_table_exists(sqlite3* db, const char* table_name);
bool util_validate_row_count(sqlite3* db, const char* table_name, int count);

