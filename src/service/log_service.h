
#pragma once


#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <nlohmann/json.hpp>

#include "log_repository.h"


using json = nlohmann::json;

class ILogService {
    public:
        virtual ~ILogService() = default;

        [[nodiscard]] virtual auto create_log(const json& body) -> bool = 0;

        [[nodiscard]] virtual auto get_logs(FilterParams params) const -> json = 0;
};

class LogService : public ILogService {
    public:
        LogService(ILogRepository& repo);

        ~LogService();

        [[nodiscard]] auto create_log(const json& body) -> bool override;

        [[nodiscard]] auto get_logs(FilterParams params) const -> json override;

    private:
        auto worker() -> void ;
        
        ILogRepository& log_repo_;
        std::thread worker_thread_;
        std::vector<LogEntry> log_buffer_;
        std::mutex mtx_;
        std::condition_variable cv_;
        bool running_ = true;

};


