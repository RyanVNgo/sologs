
#include "server.h"

#include "utils.h"


SOLogSServerDrogon::SOLogSServerDrogon(
        ILogService& log_service,
        IAuthorizer& authorizer,
        IAuthenticator& authenticator,
        IKeyService& key_service
) : log_service_(log_service),
    authorizer_(authorizer),
    authenticator_(authenticator),
    key_service_(key_service)
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
                post_auth_handler(req, std::move(callback));
            },
            {drogon::Post}
    );

}

auto SOLogSServerDrogon::start(int port) -> void {
    drogon::app()
        .setThreadNum(std::thread::hardware_concurrency())
        .addListener("127.0.0.1", port)
        .run();
}

auto SOLogSServerDrogon::stop() -> void {
    drogon::app().quit();
    std::cout << "Server stopped" << std::endl;
}

void SOLogSServerDrogon::get_health(
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

auto SOLogSServerDrogon::post_logs_handler(
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

    log_service_.create_log(body);
    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k202Accepted,
            drogon::CT_TEXT_PLAIN
    );
    resp->setBody("Accepted");
    callback(resp);
}

auto SOLogSServerDrogon::get_logs_handler(
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

    auto logs = log_service_.get_logs(params);

    auto resp = drogon::HttpResponse::newHttpResponse(
            drogon::k200OK,
            drogon::CT_APPLICATION_JSON
    );
    resp->setBody(logs.dump());
    callback(resp);
}

auto SOLogSServerDrogon::post_auth_handler(
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

    auto result = key_service_.create_key(name, permissions, expires_at);

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

auto SOLogSServerDrogon::parse_auth_key(
        const drogon::HttpRequestPtr &req
) const -> std::string {
    auto auth_header = req->getHeader("Authorization");
    if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
        return "";
    }
    return auth_header.substr(7);
}

auto SOLogSServerDrogon::authorize_user(
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

    auto subject = authenticator_.authenticate(auth_key);
    if (!subject.has_value()) {
        auto resp = drogon::HttpResponse::newHttpResponse(
                drogon::k401Unauthorized,
                drogon::CT_TEXT_PLAIN
        );
        resp->setBody("Unauthorized");
        return resp;
    }

    if (!authorizer_.has_permissions(
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

