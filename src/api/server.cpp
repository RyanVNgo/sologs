
#include "server.h"


SOLogSServer::SOLogSServer(LogService& service)
    : m_service(service)
{ }

void SOLogSServer::start(int port) {
    m_server.listen("0.0.0.0", port);
    return;
}

