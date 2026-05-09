

#include "log_repository.h"

#include <sstream>


SqlLogRepository::SqlLogRepository(SQLiteDatabase& db) 
    : m_database(db)
{
    const char* sql =
        "CREATE TABLE IF NOT EXISTS logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "message TEXT NOT NULL,"
        "level TEXT NOT NULL,"
        "source TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    m_database.execute(sql);

    return;
}

bool SqlLogRepository::insert(const LogEntry& entry) {
    const char* sql =
        "INSERT INTO logs (message, level, source)"
        "VALUES (?, ?, ?);";

    Row row_data;
    row_data.push_back(entry.message);
    row_data.push_back(entry.level);
    row_data.push_back(entry.source);

    return m_database.execute_prepared(sql, row_data);
}

bool SqlLogRepository::insert_batch(const std::vector<LogEntry>& entries) {
    std::string sql =
        "INSERT INTO logs (message, level, source)"
        "VALUES (?, ?, ?);";

    std::vector<Row> data;
    for (const auto& log : entries) {
        Row row_data;
        row_data.push_back(log.message);
        row_data.push_back(log.level);
        row_data.push_back(log.source);
        data.push_back(row_data);
    }

    return m_database.execute_prepared_batched(sql, data);
}

std::vector<LogEntry> SqlLogRepository::get_all(FilterParams params) {
    std::ostringstream sql;
    sql << "SELECT id, message, level, source, timestamp FROM logs";

    Row bound_params;

    if (params.level || params.source || params.since || params.until) {
        sql << " WHERE";
        bool first = true;

        if (params.level) {
            sql << (first ? "" : " AND") << " level = ?";
            bound_params.push_back(*params.level);
            first = false;
        }
        if (params.source) {
            sql << (first ? "" : " AND") << " source = ?";
            bound_params.push_back(*params.source);
            first = false;
        }
        if (params.since) {
            sql << (first ? "" : " AND") << " timestamp >= ?";
            bound_params.push_back(*params.since);
            first = false;
        }
        if (params.until) {
            sql << (first ? "" : " AND") << " timestamp <= ?";
            bound_params.push_back(*params.until);
            first = false;
        }
    }

    sql << " ORDER BY id DESC";

    if (params.limit) {
        sql << " LIMIT " << *params.limit;
    } else {
        sql << " LIMIT 100";
    }
    sql << ";";

    QueryResult results = m_database.query(sql.str(), bound_params);
    std::vector<LogEntry> logs;
    logs.reserve(results.size());
    for (const auto& row : results) {
        logs.push_back(
            LogEntry{
                .id = std::stoi(row.at(0)),
                .message = row.at(1),
                .level = row.at(2),
                .source = row.at(3),
                .timestamp = row.at(4)
            }
        );
    }

    return logs;
}

