
#pragma once


#include "httplib.h"

#include "log_service.h"
#include "auth_service.h"
#include "key_service.h"


class SOLogSServer {
    public:
        explicit SOLogSServer(
                ILogService& log_service,
                IAuthorizer& authorizer,
                IAuthenticator& authenticator,
                IKeyService& key_service
        );

        auto start(int port) -> void;

        auto stop() -> void;

    private:
        auto get_health_handler(
                const httplib::Request& req,
                httplib::Response& res
        ) -> void;
        
        auto post_logs_handler(
                const httplib::Request& req,
                httplib::Response& res
        ) -> void;

        auto get_logs_handler(
                const httplib::Request& req,
                httplib::Response& res
        ) -> void;

        auto post_auth_handler(
                const httplib::Request& req,
                httplib::Response& res
        ) -> void;

        auto parse_auth_key(
                const httplib::Request& req
        ) const -> std::string;

        auto authorize_user(
                const httplib::Request& req,
                httplib::Response& res,
                const std::vector<Permissions>& perms,
                const PermissionMode& mode
        ) -> bool;

        httplib::Server server_;
        ILogService& log_service_;
        IAuthorizer& authorizer_;
        IAuthenticator& authenticator_;
        IKeyService& key_service_;

};

