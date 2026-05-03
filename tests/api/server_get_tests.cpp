
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(bool, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (), (override));
};

TEST(Server, get_health) {
    LogServiceMock mock_service;
    SOLogSServer server(mock_service);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    auto res = client.Get("/health");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs) {
    LogServiceMock mock_service;
    SOLogSServer server(mock_service);

    const int port = 8080;
    std::thread t([&]() {
        server.start(port);
    });

    httplib::Client client("localhost", port);

    EXPECT_CALL(mock_service, get_logs).Times(testing::Exactly(1));
    auto res = client.Get("/logs");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}



