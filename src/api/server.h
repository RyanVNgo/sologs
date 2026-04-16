
#pragma once


#include "httplib.h"

#include "log_service.h"


class SOLogSServer {
    public:
        explicit SOLogSServer(LogService& service);

        void start(int port);

    private:
        httplib::Server m_server;
        LogService& m_service;

};

