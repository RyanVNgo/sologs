
#pragma once


#include "httplib.h"

#include "log_service.h"
#include "auth_service.h"


class SOLogSServer {
    public:
        explicit SOLogSServer(
            ILogService& service,
            IAuthorizer& authorizer,
            IAuthenticator& authenticator
        );

        void start(int port);
        void stop();

    private:
        std::string parse_auth_key(const httplib::Request& req) const;

        httplib::Server m_server;
        ILogService& m_service;
        IAuthorizer& m_authorizer;
        IAuthenticator& m_authenticator;

};

