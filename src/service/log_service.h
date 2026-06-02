
#pragma once


#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "json.hpp"

#include "log_repository.h"


using json = nlohmann::json;

class ILogService {
    public:
        virtual ~ILogService() = default;
        virtual auto create_log(const json& body) -> bool = 0;
        virtual auto get_logs(FilterParams params) -> json = 0;
};

class LogService : public ILogService {
    public:
        LogService(ILogRepository& repo);
        ~LogService();

        auto create_log(const json& body) -> bool override;
        auto get_logs(FilterParams params) -> json override;

    private:
        auto worker() -> void ;
        
        ILogRepository& m_repo;
        std::thread m_worker_thread;
        std::vector<LogEntry> m_log_buffer;
        std::mutex m_mtx;
        std::condition_variable m_cv;
        bool m_running = true;

};


