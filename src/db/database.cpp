
#include "database.h"

#include <iostream>


SQLiteDatabase::SQLiteDatabase(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to create DB" << std::endl;
    }
    return;
}

SQLiteDatabase::~SQLiteDatabase() {
    (void)sqlite3_close(m_db);
    return;
}

sqlite3* SQLiteDatabase::get() {
    return m_db;
}

bool SQLiteDatabase::execute(const std::string& query) {
    char* err_msg;

    int rc = sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &err_msg); 
    if (rc != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | SQL error: " << err_msg << "\n";
        sqlite3_free(err_msg);
        return false;
    } 

    return true;
}

bool SQLiteDatabase::execute_prepared(
        const std::string& query,
        const Row& values
) {
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << "\n";
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    for (size_t i = 0; i < values.size(); ++i) {
        int rc = sqlite3_bind_text(
                stmt,
                static_cast<int>(i + 1),
                values[i].c_str(),
                -1,
                SQLITE_TRANSIENT
        );

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    return success;
}

bool SQLiteDatabase::execute_prepared_batched(
        const std::string& query,
        const std::vector<Row>& rows
) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << "\n";
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    execute("BEGIN TRANSACTION");

    for (const auto& values : rows) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        for (size_t i = 0; i < values.size(); ++i) {
            int rc = sqlite3_bind_text(
                    stmt,
                    static_cast<int>(i + 1),
                    values[i].c_str(),
                    -1,
                    SQLITE_TRANSIENT
            );

            if (rc != SQLITE_OK) {
                sqlite3_finalize(stmt);
                return false;
            }
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    
    execute("COMMIT;");
    sqlite3_finalize(stmt);

    return true;
}

QueryResult SQLiteDatabase::query(const std::string& query) {
    QueryResult results;
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << std::endl;
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_db) << std::endl;
        return results;
    }

    int cols = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;

        for (int i = 0; i < cols; i++) {
            if (sqlite3_column_type(stmt, i) == SQLITE_NULL) {
                row.emplace_back("");
            } else {
                const unsigned char* str_value = sqlite3_column_text(stmt, i);
                row.emplace_back(reinterpret_cast<const char*>(str_value));
            }
        }

        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return results;
}

