

#include "log_repository.h"


LogRepository::LogRepository(SQLiteDatabase& db) 
    : m_database(db)
{ }

bool LogRepository::insert(const LogEntry& entry) {
    return false;
}

