
#include "server.h"


SOLogSServer::SOLogSServer(
    ILogService& log_service,
    IAuthorizer& authorizer,
    IAuthenticator& authenticator,
    IKeyService& key_service
)
    :   m_service(log_service),
        m_authorizer(authorizer),
        m_authenticator(authenticator),
        m_key_service(key_service)
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
            auto req_perms = {Permissions::LogWrite, Permissions::Admin};
            auto mode = PermissionMode::AnyOf;
            if (!authorize_user(req, res, req_perms, mode)) {
                return;
            }

            json body;
            try {
                body = json::parse(req.body);
            } catch (...) {
                res.status = 400;
                res.set_content("Invalid JSON", "text/plain");
                return;
            }

            m_service.create_log(body);
            res.status = 202;
            res.set_content("Accepted", "text/plain");
        }
    );

    m_server.Get(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res){
            auto req_perms = {Permissions::LogRead, Permissions::Admin};
            auto mode = PermissionMode::AnyOf;
            if (!authorize_user(req, res, req_perms, mode)) {
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

    m_server.Post(
        "/auth",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto req_perms = {Permissions::Admin};
            auto mode = PermissionMode::AllOf;
            if (!authorize_user(req, res, req_perms, mode)) {
                return;
            }

            json body{};
            try {
                body = json::parse(req.body);
            } catch (...) {
                res.status = 400;
                res.set_content("Invalid JSON", "text/plain");
                return;
            }

            if (!body.contains("name") || !body.contains("permissions")) {
                res.status = 400;
                res.set_content("Missing required fields: name, permissions", "text/plain");
                return;
            }

            std::string name = body["name"];

            std::vector<Permissions> permissions;
            for (const auto& p : body["permissions"]) {
                std::string label = p;
                if (label == "LogRead")         permissions.push_back(Permissions::LogRead);
                else if (label == "LogWrite")   permissions.push_back(Permissions::LogWrite);
                else if (label == "LogDelete")  permissions.push_back(Permissions::LogDelete);
                else if (label == "AuthRead")   permissions.push_back(Permissions::AuthRead);
                else if (label == "AuthWrite")  permissions.push_back(Permissions::AuthWrite);
                else if (label == "AuthDelete") permissions.push_back(Permissions::AuthDelete);
                else if (label == "Admin")      permissions.push_back(Permissions::Admin);
            }

            std::string expires_at = "9999-12-31 23:59:59";
            if (body.contains("expires_at")) {
                expires_at = body["expires_at"];
            }

            auto result = m_key_service.create_key(name, permissions, expires_at);

            json resp;
            resp["key"] = result.raw_key;
            resp["uuid"] = result.entry.uuid;
            resp["name"] = result.entry.name;
            resp["created_at"] = result.entry.created_at;
            resp["expires_at"] = result.entry.expires_at;
            resp["is_valid"] = result.entry.is_valid;

            json perms = json::array();
            for (auto perm : result.entry.permissions) {
                switch (perm) {
                    case Permissions::LogRead:    perms.push_back("LogRead"); break;
                    case Permissions::LogWrite:   perms.push_back("LogWrite"); break;
                    case Permissions::LogDelete:  perms.push_back("LogDelete"); break;
                    case Permissions::AuthRead:   perms.push_back("AuthRead"); break;
                    case Permissions::AuthWrite:  perms.push_back("AuthWrite"); break;
                    case Permissions::AuthDelete: perms.push_back("AuthDelete"); break;
                    case Permissions::Admin:      perms.push_back("Admin"); break;
                }
            }
            resp["permissions"] = perms;

            res.status = 201;
            res.set_content(resp.dump(), "application/json");
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

bool SOLogSServer::authorize_user(
    const httplib::Request& req,
    httplib::Response& res,
    const std::vector<Permissions>& perms,
    const PermissionMode& mode
) {
    std::string auth_key = parse_auth_key(req);
    if (auth_key.empty()) {
        res.status = 401;
        res.set_content("Unauthorized", "text/plain");
        return false;
    }

    auto subject = m_authenticator.authenticate(auth_key);
    if (!subject.has_value()) {
        res.status = 401;
        res.set_content("Unauthorized", "text/plain");
        return false;
    }

    if (!m_authorizer.has_permissions(
            subject.value(),
            perms,
            mode
    )) {
        res.status = 403;
        res.set_content("Forbidden", "text/plain");
        return false;
    }

    return true;
}

