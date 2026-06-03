
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
        
        ILogRepository& log_repo_;
        std::thread worker_thread_;
        std::vector<LogEntry> log_buffer_;
        std::mutex mtx_;
        std::condition_variable cv_;
        bool running_ = true;

};


