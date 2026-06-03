
#include "server.h"

#include "utils.h"


SOLogSServer::SOLogSServer(
        ILogService& log_service,
        IAuthorizer& authorizer,
        IAuthenticator& authenticator,
        IKeyService& key_service
) : log_service_(log_service),
    authorizer_(authorizer),
    authenticator_(authenticator),
    key_service_(key_service)
{
    server_.Get(
        "/health",
        [this](const httplib::Request& req, httplib::Response& res) {
            this->get_health_handler(req, res);
        }
    );

    server_.Post(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res) {
            this->post_logs_handler(req, res);
        }
    );

    server_.Get(
        "/logs",
        [this](const httplib::Request& req, httplib::Response& res){
            this->get_logs_handler(req, res);
        }
    );

    server_.Post(
        "/auth",
        [this](const httplib::Request& req, httplib::Response& res) {
            this->post_auth_handler(req, res);
        }
    );

}

auto SOLogSServer::start(int port) -> void {
    server_.listen("127.0.0.1", port);
}

auto SOLogSServer::stop() -> void {
    server_.wait_until_ready();
    server_.stop();
    std::cout << "Server stopped" << std::endl;
}

auto SOLogSServer::get_health_handler(
        const httplib::Request& req,
        httplib::Response& res
) -> void {
    res.status = httplib::OK_200;
    res.set_content("OK", "text/plain");
}

auto SOLogSServer::post_logs_handler(
        const httplib::Request& req,
        httplib::Response& res
) -> void {
    auto req_perms = {Permissions::LogWrite, Permissions::Admin};
    auto mode = PermissionMode::AnyOf;
    if (!authorize_user(req, res, req_perms, mode)) {
        return;
    }

    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = httplib::BadRequest_400;
        res.set_content("Invalid JSON", "text/plain");
        return;
    }

    log_service_.create_log(body);
    res.status = httplib::Accepted_202;
    res.set_content("Accepted", "text/plain");
}

auto SOLogSServer::get_logs_handler(
        const httplib::Request& req,
        httplib::Response& res
) -> void {
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

    auto logs = log_service_.get_logs(params);
    res.status = httplib::OK_200;
    res.set_content(logs.dump(), "application/json");
}

auto SOLogSServer::post_auth_handler(
        const httplib::Request& req,
        httplib::Response& res
) -> void {
    auto req_perms = {Permissions::Admin};
    auto mode = PermissionMode::AllOf;
    if (!authorize_user(req, res, req_perms, mode)) {
        return;
    }

    json body{};
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = httplib::BadRequest_400;
        res.set_content("Invalid JSON", "text/plain");
        return;
    }

    if (!body.contains("name") || !body.contains("permissions")) {
        res.status = httplib::BadRequest_400;
        res.set_content("Missing required fields: name, permissions", "text/plain");
        return;
    }

    std::string name = body["name"];

    std::vector<Permissions> permissions;
    for (const auto& p : body["permissions"]) {
        if (auto perm = sologs::utils::permission_from_label(p)) {
            permissions.push_back(perm.value());
        }
    }

    std::string expires_at = "9999-12-31 23:59:59";
    if (body.contains("expires_at")) {
        expires_at = body["expires_at"];
    }

    auto result = key_service_.create_key(name, permissions, expires_at);

    json resp;
    resp["key"] = result.raw_key;
    resp["uuid"] = result.entry.uuid;
    resp["name"] = result.entry.name;
    resp["created_at"] = result.entry.created_at;
    resp["expires_at"] = result.entry.expires_at;
    resp["is_valid"] = result.entry.is_valid;

    json perms = json::array();
    for (auto perm : result.entry.permissions) {
        perms.push_back(sologs::utils::permission_label(perm));
    }
    resp["permissions"] = perms;

    res.status = httplib::Created_201;
    res.set_content(resp.dump(), "application/json");
}

auto SOLogSServer::parse_auth_key(
        const httplib::Request& req
) const -> std::string {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
        return "";
    }
    return auth_header.substr(7);
}


auto SOLogSServer::authorize_user(
        const httplib::Request& req,
        httplib::Response& res,
        const std::vector<Permissions>& perms,
        const PermissionMode& mode
) -> bool {
    std::string auth_key = parse_auth_key(req);
    if (auth_key.empty()) {
        res.status = httplib::Unauthorized_401;
        res.set_content("Unauthorized", "text/plain");
        return false;
    }

    auto subject = authenticator_.authenticate(auth_key);
    if (!subject.has_value()) {
        res.status = httplib::Unauthorized_401;
        res.set_content("Unauthorized", "text/plain");
        return false;
    }

    if (!authorizer_.has_permissions(
            subject.value(),
            perms,
            mode
    )) {
        res.status = httplib::Forbidden_403;
        res.set_content("Forbidden", "text/plain");
        return false;
    }

    return true;
}

