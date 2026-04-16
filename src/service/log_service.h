
#pragma once


#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "json.hpp"

#include "log_repository.h"


using json = nlohmann::json;

class LogService {
    public:
        LogService(LogRepository& repo);
        ~LogService();

        bool create_log(const json& body);
        json get_logs();

    private:
        LogRepository& m_repo;
        std::thread m_worker_thread;
        std::queue<LogEntry> m_log_queue;
        std::mutex m_mtx;
        std::condition_variable m_cv;
        bool m_running = true;

        void worker();
        
};


