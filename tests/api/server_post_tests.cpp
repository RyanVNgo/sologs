
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(bool, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (FilterParams params), (override));
};

TEST(Server, post_valid) {
    LogServiceMock mock_service;
    SOLogSServer server(mock_service);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(1));
    auto res = client.Post(
            "/logs", 
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
                    "source": "sologs-benchmark"
                }
            )",
            "application/json"
    );

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 202);

    server.stop();
    t.join();
}

TEST(Server, post_invalid) {
    LogServiceMock mock_service;
    SOLogSServer server(mock_service);

    const int port = 8080;
    std::thread t([&]() {
        server.start(port);
    });

    httplib::Client client("localhost", port);

    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(0));
    auto res = client.Post(
            "/logs", 
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
            )",
            "application/json"
    );

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);

    server.stop();
    t.join();
}


