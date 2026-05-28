
#include "server.h"


SOLogSServer::SOLogSServer(
    ILogService& service,
    IAuthorizer& authorizer,
    IAuthenticator& authenticator
)
    :   m_service(service),
        m_authorizer(authorizer),
        m_authenticator(authenticator)
{
    m_server.Get(
        "/health",
        [this](const httplib::Request& req, httplib::Response& res) {
            res.status = 200;
            res.set_content("OK", "text/plain");
        }
    );

    m_server.Post(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res) {
            std::string key = parse_auth_key(req);
            if (key.empty()) {
                res.status = 401;
                res.set_content("Unauthorized", "text/plain");
                return;
            }

            auto subject = m_authenticator.authenticate(key);
            if (!subject.has_value()) {
                res.status = 401;
                res.set_content("Unauthorized", "text/plain");
                return;
            }

            bool has_permissions = m_authorizer.has_permissions(
                    subject.value(),
                    {Permissions::LogWrite, Permissions::Admin},
                    PermissionMode::AnyOf
            );

            if (!has_permissions) {
                res.status = 403;
                res.set_content("Forbidden", "text/plain");
                return;
            }

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
            std::string key = parse_auth_key(req);
            if (key.empty()) {
                res.status = 401;
                res.set_content("Unauthorized", "text/plain");
                return;
            }

            auto subject = m_authenticator.authenticate(key);
            if (!subject.has_value()) {
                res.status = 401;
                res.set_content("Unauthorized", "text/plain");
                return;
            }

            bool has_permissions = m_authorizer.has_permissions(
                    subject.value(),
                    {Permissions::LogRead, Permissions::Admin},
                    PermissionMode::AnyOf
            );

            if (!has_permissions) {
                res.status = 403;
                res.set_content("Forbidden", "text/plain");
                return;
            }

            FilterParams params;

            if (auto it = req.params.find("level"); it != req.params.end()) {
                params.level = it->second;
            }
            if (auto it = req.params.find("source"); it != req.params.end()) {
                params.source = it->second;
            }
            if (auto it = req.params.find("since"); it != req.params.end()) {
                params.since = it->second;
            }
            if (auto it = req.params.find("until"); it != req.params.end()) {
                params.until = it->second;
            }
            if (auto it = req.params.find("limit"); it != req.params.end()) {
                try {
                    params.limit = std::stoi(it->second);
                } catch (...) {
                }
            }

            auto logs = m_service.get_logs(params);
            res.status = 200;
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

std::string SOLogSServer::parse_auth_key(const httplib::Request& req) const {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
        return "";
    }
    return auth_header.substr(7);
}

