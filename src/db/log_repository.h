
#pragma once


#include "database.h"
#include "log_entry.h"


class ILogRepository {
    public:
        virtual ~ILogRepository() = default;

        virtual auto insert(const LogEntry& entry) -> void = 0;

        virtual auto insert_batch(
                const std::vector<LogEntry>& entries
        ) -> void = 0;

        [[nodiscard]]
        virtual auto get_all(
                LogFilterParams params
        ) const -> std::vector<LogEntry> = 0;
};

class SqlLogRepository : public ILogRepository {
    public:
        SqlLogRepository(SQLiteDatabase& db);

        auto insert(const LogEntry& entry) -> void override;

        auto insert_batch(
                const std::vector<LogEntry>& entries
        ) -> void override;

        [[nodiscard]]
        auto get_all(
                LogFilterParams params
        ) const -> std::vector<LogEntry> override;

    private:
        SQLiteDatabase& database_;

};

