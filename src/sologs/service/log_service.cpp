
#include <service/log_service.h>


LogService::LogService(
        ILogRepository& repo
) : log_repo_(repo),
    worker_thread_(&LogService::worker, this)
{ }

LogService::~LogService() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        running_ = false;
    }
    cv_.notify_all();
    worker_thread_.join();
}

auto LogService::create_log(const json& body) -> void {
    if (!body.contains("message") || 
        !body.contains("level") || 
        !body.contains("source")
    ) {
        throw std::invalid_argument(
            "Missing required fields: message, level, source"
        );
    }

    LogEntry new_entry{
        .message = body["message"],
        .level = body["level"],
        .source = body["source"]
    };

    bool was_empty;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        was_empty = log_buffer_.empty();
        log_buffer_.push_back(std::move(new_entry));
    }

    if (was_empty) {
        cv_.notify_one();
    }
}

auto LogService::get_logs(LogFilterParams params) const -> json {
    std::vector<LogEntry> logs = log_repo_.get_all(params);
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

auto LogService::worker() -> void {
    std::vector<LogEntry> batch;
    const size_t max_batch_size = 1024 * 4;
    batch.reserve(max_batch_size);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(
            lock, 
            [&] {
                return !log_buffer_.empty() || !running_;
            }
        );
        
        if (!running_ && log_buffer_.empty()) {
            break;
        }

        if (log_buffer_.size() > max_batch_size) {
            batch.insert(
                    batch.begin(),
                    log_buffer_.begin(),
                    log_buffer_.begin() + max_batch_size
            );
            log_buffer_.erase(
                    log_buffer_.begin(),
                    log_buffer_.begin() + max_batch_size
            );
        } else {
            batch.insert(
                    batch.begin(),
                    log_buffer_.begin(),
                    log_buffer_.end()
            );
            log_buffer_.clear();
        }

        lock.unlock();

        if (!batch.empty()) {
            try {
                log_repo_.insert_batch(batch);
            } catch (const std::exception&) {
            }
            batch.clear();
        }

    }
}

