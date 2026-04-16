
#pragma once


#include "json.hpp"

#include "database.h"
#include "log_entry.h"


using json = nlohmann::json;

class LogRepository {
    public:
        LogRepository(SQLiteDatabase& db);

        bool insert(const LogEntry& entry);

    private:
        SQLiteDatabase& m_database;

};

