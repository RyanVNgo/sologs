
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
        virtual ~ILogService() {};
        virtual bool create_log(const json& body) = 0;
        virtual json get_logs() = 0;
};

class LogService : public ILogService {
    public:
        LogService(ILogRepository& repo);
        ~LogService();

        bool create_log(const json& body) override;
        json get_logs() override;

    private:
        ILogRepository& m_repo;
        std::thread m_worker_thread;
        std::vector<LogEntry> m_log_buffer;
        std::mutex m_mtx;
        std::condition_variable m_cv;
        bool m_running = true;

        void worker();
        
};


