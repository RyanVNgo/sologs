
#pragma once


#include "database.h"
#include "log_entry.h"


class LogRepository {
    public:
        LogRepository(SQLiteDatabase& db);

        bool insert(const LogEntry& entry);
        bool insert_batch(const std::vector<LogEntry>& entries);
        std::vector<LogEntry> get_all();

    private:
        SQLiteDatabase& m_database;

};

