
#include "log_service.h"

#include <iostream>


LogService::LogService(LogRepository& repo)
    : m_repo(repo),
      m_worker_thread(&LogService::worker, this)
{ }

LogService::~LogService() {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_running = false;
    }
    m_cv.notify_all();
    m_worker_thread.join();
}

bool LogService::create_log(const json& body) {
    if (!body.contains("message") || !body.contains("level") || !body.contains("source")) {
        return false;
    }

    LogEntry new_entry{
        .message = body["message"],
        .level = body["level"],
        .source = body["source"]
    };

    bool was_empty;
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        was_empty = m_log_buffer.empty();
        m_log_buffer.push_back(std::move(new_entry));
    }

    if (was_empty) {
        m_cv.notify_one();
    }

    return true;
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

void LogService::worker() {
    std::vector<LogEntry> batch;
    const size_t max_batch_size = 1024 * 4;
    batch.reserve(max_batch_size);

    while (true) {
        std::unique_lock<std::mutex> lock(m_mtx);

        m_cv.wait(
            lock, 
            [&] {
                return !m_log_buffer.empty() || !m_running;
            }
        );
        
        if (!m_running && m_log_buffer.empty()) {
            break;
        }

        if (m_log_buffer.size() > max_batch_size) {
            batch.insert(
                    batch.begin(),
                    m_log_buffer.begin(),
                    m_log_buffer.begin() + max_batch_size
            );
            m_log_buffer.erase(
                    m_log_buffer.begin(),
                    m_log_buffer.begin() + max_batch_size
            );
        } else {
            batch.insert(
                    batch.begin(),
                    m_log_buffer.begin(),
                    m_log_buffer.end()
            );
            m_log_buffer.clear();
        }

        lock.unlock();

        if (!batch.empty()) {
            m_repo.insert_batch(batch);
            batch.clear();
        }

    }
}

