

#include "log_repository.h"


LogRepository::LogRepository(SQLiteDatabase& db) 
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

bool LogRepository::insert(const LogEntry& entry) {
    const char* sql =
        "INSERT INTO logs (message, level, source)"
        "VALUES (?, ?, ?);";

    Row row_data;
    row_data.push_back(entry.message);
    row_data.push_back(entry.level);
    row_data.push_back(entry.source);

    return m_database.execute_prepared(sql, row_data);
}

bool LogRepository::insert_batch(const std::vector<LogEntry>& entries) {
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

std::vector<LogEntry> LogRepository::get_all() {
    const char* sql = 
        "SELECT id, message, level, source, timestamp FROM logs "
        "ORDER BY id DESC "
        "LIMIT 100;";

    QueryResult results = m_database.query(sql);
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

