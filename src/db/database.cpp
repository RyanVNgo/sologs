
#include "database.h"

#include <iostream>

#include <sqlite3.h>


struct SQLiteDatabase::Impl {
    sqlite3* db;
};

SQLiteDatabase::SQLiteDatabase(const std::string& db_path) 
    : m_pimpl(new Impl())
{
    if (sqlite3_open(db_path.c_str(), &(m_pimpl->db)) != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(m_pimpl->db);
        sqlite3_close(m_pimpl->db);
        throw std::runtime_error("Failed to open database: " + msg);
    }
}

SQLiteDatabase::~SQLiteDatabase() {
    if (m_pimpl->db) {
        sqlite3_close(m_pimpl->db);
    }
}

bool SQLiteDatabase::execute(const std::string& query) {
    int rc = sqlite3_exec(m_pimpl->db, query.c_str(), nullptr, nullptr, nullptr); 
    if (rc != SQLITE_OK) {
        return false;
    } 
    return true;
}

bool SQLiteDatabase::execute_prepared(
        const std::string& query,
        const Row& values
) {
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pimpl->db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << "\n";
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_pimpl->db) << std::endl;
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

    if (sqlite3_prepare_v2(m_pimpl->db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << "\n";
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_pimpl->db) << std::endl;
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
    
    if (sqlite3_prepare_v2(m_pimpl->db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << std::endl;
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_pimpl->db) << std::endl;
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

QueryResult SQLiteDatabase::query(const std::string& query, const Row& params) {
    QueryResult results;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pimpl->db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLiteDatabase | Failed to prepare stmt" << std::endl;
        std::cerr << "  SQL: " << query << "\n";
        std::cout << "  Error: " << sqlite3_errmsg(m_pimpl->db) << std::endl;
        return results;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return results;
        }
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

