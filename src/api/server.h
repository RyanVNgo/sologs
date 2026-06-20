
#pragma once


#include <drogon/HttpAppFramework.h>
#include <drogon/HttpController.h>

#include "log_service.h"
#include "user_service.h"


class SOLogSServer {
    public:
        explicit SOLogSServer(
                ILogService& log_service,
                IUserService& auth_service
        );

        auto start(int port) -> void;

        auto stop() -> void;

        auto get_health(
                const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) -> void;

        auto post_logs_handler(
                const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) -> void;

        auto get_logs_handler(
                const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) -> void;

        auto post_auth_handler(
                const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) -> void;

        auto get_auth_handler(
                const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) -> void;

    private:
        auto parse_auth_key(
                const drogon::HttpRequestPtr& req
        ) const -> std::string;

        auto authorize_user(
                const drogon::HttpRequestPtr& req,
                const PermissionList& perms,
                const PermissionMode& mode
        ) -> std::optional<drogon::HttpResponsePtr>;

        ILogService& log_service_;
        IUserService& auth_service_;

};

