
#include "db_utils.h"


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

