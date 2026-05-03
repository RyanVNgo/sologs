
#pragma once


#include "httplib.h"

#include "log_service.h"


class SOLogSServer {
    public:
        explicit SOLogSServer(ILogService& service);

        void start(int port);
        void stop();

    private:
        httplib::Server m_server;
        ILogService& m_service;

};

