
#include "db_utils.h"

#include <format>


SqliteHandle util_open_database(const char* path) {
    sqlite3* db = nullptr;

    if (sqlite3_open(path, &db) != SQLITE_OK) {
        std::string msg = db ? sqlite3_errmsg(db) : "unknown error";
        if (db) {
            sqlite3_close(db);
        }
        throw std::runtime_error("util_open_database failed: " + msg);
    }
    
    return SqliteHandle(db, sqlite3_close);
}

bool util_exec_query(sqlite3* db, const char* query) {
    int rc = sqlite3_exec(
            db,
            query,
            nullptr,
            nullptr,
            nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    return true;
}

bool util_insert_log(
        sqlite3* db,
        const std::string& message,
        const std::string& level,
        const std::string& source,
        const std::string& timestamp
) {
    std::string query;
    if (timestamp.empty()) {
        query = std::format(R"(
            INSERT INTO logs (message, level, source)
            VALUES ('{}', '{}', '{}');
            )",
            message, level, source
        );
    } else {
        query = std::format(R"(
            INSERT INTO logs (message, level, source, timestamp)
            VALUES ('{}', '{}', '{}', '{}');
            )",
            message, level, source, timestamp
        );
    }
    return util_exec_query(db, query.c_str());
}

