
#include "log_service.h"


LogService::LogService(LogRepository& repo)
    : m_repo(repo)
{ }

bool LogService::create_log(const json& body) {
    if (!body.contains("message") || !body.contains("level")) {
        return false;
    }

    LogEntry new_entry{
        .message = body["message"],
        .level = body["level"],
        .source = body["source"]
    };

    return m_repo.insert(new_entry);
}

json LogService::get_logs() {
    std::vector<LogEntry> logs = m_repo.get_all();
    json arr = json::array();

    for (auto& log : logs) {
        arr.push_back(
            {
                {"id", log.id},
                {"message", log.message},
                {"level", log.level},
                {"source", log.source},
                {"timestamp", log.timestamp}
            }
        );
    }

    return arr;
}

