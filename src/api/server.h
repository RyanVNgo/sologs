
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

        void start(int port);
        void stop();

    private:
        std::string parse_auth_key(const httplib::Request& req) const;
        bool authorize_user(
            const httplib::Request& req,
            httplib::Response& res,
            const std::vector<Permissions>& perms,
            const PermissionMode& mode
        );

        httplib::Server m_server;
        ILogService& m_service;
        IAuthorizer& m_authorizer;
        IAuthenticator& m_authenticator;
        IKeyService& m_key_service;

};

