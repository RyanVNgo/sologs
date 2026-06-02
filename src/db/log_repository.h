
#pragma once


#include "database.h"
#include "log_entry.h"


class ILogRepository {
    public:
        virtual ~ILogRepository() = default;
        virtual auto insert(const LogEntry& entry) -> bool = 0;
        virtual auto insert_batch(const std::vector<LogEntry>& entries) -> bool = 0;
        virtual auto get_all(FilterParams params) -> std::vector<LogEntry> = 0;
};

class SqlLogRepository : public ILogRepository {
    public:
        SqlLogRepository(SQLiteDatabase& db);

        auto insert(const LogEntry& entry) -> bool override;
        auto insert_batch(const std::vector<LogEntry>& entries) -> bool override;
        auto get_all(FilterParams params) -> std::vector<LogEntry> override;

    private:
        SQLiteDatabase& m_database;

};

