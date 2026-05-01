
#pragma once


#include "database.h"
#include "log_entry.h"


class ILogRepository {
    public:
        virtual ~ILogRepository() {};
        virtual bool insert(const LogEntry& entry) = 0;
        virtual bool insert_batch(const std::vector<LogEntry>& entries) = 0;
        virtual std::vector<LogEntry> get_all() = 0;
};

class SqlLogRepository : public ILogRepository {
    public:
        SqlLogRepository(SQLiteDatabase& db);

        bool insert(const LogEntry& entry) override;
        bool insert_batch(const std::vector<LogEntry>& entries) override;
        std::vector<LogEntry> get_all() override;

    private:
        SQLiteDatabase& m_database;

};

