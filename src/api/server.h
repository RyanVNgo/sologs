
#pragma once


#include <drogon/HttpAppFramework.h>
#include <drogon/HttpController.h>

#include "log_service.h"
#include "auth_service.h"
#include "key_service.h"


class SOLogSServerDrogon {
    public:
        explicit SOLogSServerDrogon(
                ILogService& log_service,
                IAuthorizer& authorizer,
                IAuthenticator& authenticator,
                IKeyService& key_service
        );

        auto start(int port) -> void;

        auto stop() -> void;

        auto get_health(
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
        ) -> void;

        auto post_logs_handler(
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
        ) -> void;

        auto get_logs_handler(
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
        ) -> void;

        auto post_auth_handler(
                const drogon::HttpRequestPtr &req,
                std::function<void (const drogon::HttpResponsePtr &)> &&callback
        ) -> void;

    private:
        auto parse_auth_key(
                const drogon::HttpRequestPtr &req
        ) const -> std::string;

        auto authorize_user(
                const drogon::HttpRequestPtr &req,
                const std::vector<Permissions>& perms,
                const PermissionMode& mode
        ) -> std::optional<drogon::HttpResponsePtr>;

        ILogService& log_service_;
        IAuthorizer& authorizer_;
        IAuthenticator& authenticator_;
        IKeyService& key_service_;

};

