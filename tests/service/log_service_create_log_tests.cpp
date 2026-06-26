
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <service/log_service.h>


class LogRepositoryMock : public ILogRepository {
    public:
        MOCK_METHOD(void, insert, (const LogEntry& entry), (override));
        MOCK_METHOD(void, insert_batch, (const std::vector<LogEntry>& entries), (override));
        MOCK_METHOD(std::vector<LogEntry>, get_all, (LogFilterParams params), (const override));
};

TEST(LogService, create_log_valid) {
    LogRepositoryMock mock_repo;
    LogService service(mock_repo);

    LogEntry log_data = {
        .message = "test message",
        .level = "INFO",
        .source = "log service tests",
    };

    json valid_log = {
        {"message", log_data.message},
        {"level", log_data.level},
        {"source", log_data.source},
    };

    EXPECT_CALL(mock_repo, insert_batch).Times(testing::Exactly(1));
    EXPECT_NO_THROW(service.create_log(valid_log));
}

TEST(LogService, create_log_invalid) {
    LogRepositoryMock mock_repo;
    LogService service(mock_repo);

    LogEntry log_data = {
        .message = "test message",
        .level = "INFO",
        .source = "log service tests",
    };

    std::vector<json> invalid_logs = {
        { // missing message field 
            {"level", log_data.level},
            {"source", log_data.source}
        },
        { // missing level field 
            {"message", log_data.message},
            {"source", log_data.source}
        },
        { // missing source field 
            {"message", log_data.message},
            {"level", log_data.level}
        }
    };

    EXPECT_CALL(mock_repo, insert_batch).Times(testing::Exactly(0));

    for (const auto& log: invalid_logs) {
        EXPECT_THROW(service.create_log(log), std::invalid_argument);
    }
}

