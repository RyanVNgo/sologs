
#include "server.h"

#include "utils.h"


SOLogSServer::SOLogSServer(
        ILogService& log_service,
        IAuthService& auth_service
) : log_service_(log_service),
    auth_service_(auth_service)
{
    drogon::app().registerHandler(
            "/health",
            [this] (
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
            ) { get_health(req, std::move(callback)); },
            {drogon::Get}
    );

    drogon::app().registerHandler(
            "/logs",
            [this] (
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
            ) { 
                if (req->method() == drogon::Post) {
                    post_logs_handler(req, std::move(callback)); 
                } else if (req->method() == drogon::Get) {
                    get_logs_handler(req, std::move(callback));
                }
            },
            {drogon::Post, drogon::Get}
    );

    drogon::app().registerHandler(
            "/auth",
            [this](
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
            ) {
                if (req->method() == drogon::Post) {
                    post_auth_handler(req, std::move(callback));
                } else if (req->method() == drogon::Get) {
                    get_auth_handler(req, std::move(callback));
                }
            },
            {drogon::Post, drogon::Get}
    );

}

auto SOLogSServer::start(int port) -> void {
    if (drogon::app().isRunning()) {
        return;
    }
    drogon::app()
        .setThreadNum(std::thread::hardware_concurrency())
        .addListener("127.0.0.1", port)
        .run();
}

auto SOLogSServer::stop() -> void {
    if (!drogon::app().isRunning()) {
        return;
    }
    drogon::app().quit();
    std::cout << "Server stopped" << std::endl;
}

void SOLogSServer::get_health(
        const drogon::HttpRequestPtr &req,
        std::function<void (const drogon::HttpResponsePtr &)> &&callback
) {
    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k200OK,
            drogon::CT_TEXT_PLAIN
    );
    resp->setBody("OK");
    callback(resp);
}

auto SOLogSServer::post_logs_handler(
        const drogon::HttpRequestPtr &req,
        std::function<void (const drogon::HttpResponsePtr &)> &&callback
) -> void {
    static const auto req_perms = {
        Permissions::LogWrite, 
        Permissions::Admin
    };
    auto mode = PermissionMode::AnyOf;
    auto auth_resp = authorize_user(req, req_perms, mode);
    if (auth_resp.has_value()) {
        callback(auth_resp.value());
        return;
    }

    json body{};
    try {
        body = json::parse(req->body());
    } catch (const json::parse_error&) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k400BadRequest,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Invalid JSON");
        callback(resp);
        return;
    } catch (...) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k500InternalServerError,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Internal Server Error");
        callback(resp);
        return;
    }

    try {
        log_service_.create_log(body);
    } catch (const std::invalid_argument&) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k400BadRequest,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Missing required fields");
        callback(resp);
        return;
    }
    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k202Accepted,
            drogon::CT_TEXT_PLAIN
    );
    resp->setBody("Accepted");
    callback(resp);
}

auto SOLogSServer::get_logs_handler(
        const drogon::HttpRequestPtr &req,
        std::function<void (const drogon::HttpResponsePtr &)> &&callback
) -> void {
    static const auto req_perms = {
        Permissions::LogRead, 
        Permissions::Admin
    };
    auto mode = PermissionMode::AnyOf;
    auto auth_resp = authorize_user(req, req_perms, mode);
    if (auth_resp.has_value()) {
        callback(auth_resp.value());
        return;
    }

    FilterParams params;

    if (auto it = req->parameters().find("level"); it != req->parameters().end()) {
        params.level = it->second;
    }
    if (auto it = req->parameters().find("source"); it != req->parameters().end()) {
        params.source = it->second;
    }
    if (auto it = req->parameters().find("since"); it != req->parameters().end()) {
        params.since = it->second;
    }
    if (auto it = req->parameters().find("until"); it != req->parameters().end()) {
        params.until = it->second;
    }
    if (auto it = req->parameters().find("limit"); it != req->parameters().end()) {
        try {
            params.limit = std::stoi(it->second);
        } catch (...) {
        }
    }

    json logs;
    try {
        logs = log_service_.get_logs(params);
    } catch (const std::exception&) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k500InternalServerError,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Internal Server Error");
        callback(resp);
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k200OK,
            drogon::CT_APPLICATION_JSON
    );
    resp->setBody(logs.dump());
    callback(resp);
}

auto SOLogSServer::post_auth_handler(
        const drogon::HttpRequestPtr &req,
        std::function<void (const drogon::HttpResponsePtr &)> &&callback
) -> void {
    static const auto req_perms = {Permissions::Admin};
    auto mode = PermissionMode::AllOf;
    auto auth_resp = authorize_user(req, req_perms, mode);
    if (auth_resp.has_value()) {
        callback(auth_resp.value());
        return;
    }

    json body{};
    try {
        body = json::parse(req->body());
    } catch (...) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k400BadRequest,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Unauthorized");
        callback(resp);
        return;
    }

    if (!body.contains("name") || !body.contains("permissions")) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k400BadRequest,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Missing required fields: name, permissions");
        callback(resp);
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

    IAuthService::CreateUserResult result;
    try {
        result = auth_service_.create_user(name, permissions, expires_at);
    } catch (const std::exception&) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k500InternalServerError,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Internal Server Error");
        callback(resp);
        return;
    }

    json data_resp;
    data_resp["key"] = result.raw_key;
    data_resp["uuid"] = result.entry.uuid;
    data_resp["name"] = result.entry.name;
    data_resp["created_at"] = result.entry.created_at;
    data_resp["expires_at"] = result.entry.expires_at;
    data_resp["is_valid"] = result.entry.is_valid;

    json perms = json::array();
    for (auto perm : result.entry.permissions) {
        perms.push_back(sologs::utils::permission_label(perm));
    }
    data_resp["permissions"] = perms;

    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k201Created,
            drogon::CT_APPLICATION_JSON
    );
    resp->setBody(data_resp.dump());
    callback(resp);
}

auto SOLogSServer::get_auth_handler(
        const drogon::HttpRequestPtr &req,
        std::function<void (const drogon::HttpResponsePtr &)> &&callback
) -> void {
    static const auto req_perms = {
        Permissions::Admin,
        Permissions::AuthRead
    };
    auto mode = PermissionMode::AnyOf;
    auto auth_resp = authorize_user(req, req_perms, mode);
    if (auth_resp.has_value()) {
        callback(auth_resp.value());
        return;
    }

    UserFilterParams params{.limit = 100};

    if (auto it = req->parameters().find("uuid"); it != req->parameters().end()) {
        params.uuid= it->second;
    }
    if (auto it = req->parameters().find("name"); it != req->parameters().end()) {
        params.name = it->second;
    }
    if (auto it = req->parameters().find("permissions"); it != req->parameters().end()) {
        params.permissions = sologs::utils::parse_permissions(it->second);
    }
    if (auto it = req->parameters().find("created_after"); it != req->parameters().end()) {
        params.created_after= it->second;
    }
    if (auto it = req->parameters().find("created_before"); it != req->parameters().end()) {
        params.created_before= it->second;
    }
    if (auto it = req->parameters().find("expires_after"); it != req->parameters().end()) {
        params.expires_after = it->second;
    }
    if (auto it = req->parameters().find("expires_before"); it != req->parameters().end()) {
        params.expires_before = it->second;
    }
    if (auto it = req->parameters().find("is_valid"); it != req->parameters().end()) {
        params.is_valid = it->second == "false" ? false : true;
    }
    if (auto it = req->parameters().find("limit"); it != req->parameters().end()) {
        try {
            params.limit = std::stoi(it->second);
        } catch (...) {
        }
    }

    json users;
    try {
        users = auth_service_.get_users(params);
    } catch (const std::invalid_argument& e) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k400BadRequest,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody(e.what());
        callback(resp);
        return;
    } catch (const std::exception&) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k500InternalServerError,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Internal Server Error");
        callback(resp);
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k200OK,
            drogon::CT_APPLICATION_JSON
    );
    resp->setBody(users.dump());
    callback(resp);
}

auto SOLogSServer::parse_auth_key(
        const drogon::HttpRequestPtr &req
) const -> std::string {
    auto auth_header = req->getHeader("Authorization");
    if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
        return "";
    }
    return auth_header.substr(7);
}

auto SOLogSServer::authorize_user(
        const drogon::HttpRequestPtr &req,
        const std::vector<Permissions>& perms,
        const PermissionMode& mode
) -> std::optional<drogon::HttpResponsePtr> {
    std::string auth_key = parse_auth_key(req);
    if (auth_key.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k401Unauthorized,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Unauthorized");
        return resp;
    }

    auto subject = auth_service_.authenticate(auth_key);
    if (!subject.has_value()) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k401Unauthorized,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Unauthorized");
        return resp;
    }

    if (!auth_service_.subject_has_permissions(
            subject.value(),
            perms,
            mode
    )) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k403Forbidden,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Forbidden");
        return resp;
    }

    return std::nullopt;
}

