
#include "server.h"


SOLogSServer::SOLogSServer(ILogService& service)
    : m_service(service)
{
    m_server.Get(
        "/health",
        [](const httplib::Request& req, httplib::Response& res){
            res.status = 200;
            res.set_content("OK", "text/plain");
        }
    );

    m_server.Post(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = json::parse(req.body);
                m_service.create_log(body);
                res.status = 202;
                res.set_content("Accepted", "text/plain");
            } catch (...) {
                res.status = 400;
                res.set_content("Invalid JSON", "text/plain");
            }
        }
    );

    m_server.Get(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res){
            auto logs = m_service.get_logs();
            res.set_content(logs.dump(), "application/json");
        }
    );
}

void SOLogSServer::start(int port) {
    m_server.listen("127.0.0.1", port);
}

void SOLogSServer::stop() {
    m_server.wait_until_ready();
    m_server.stop();
    std::cout << "Server stopped" << std::endl;
}

