
#pragma once

#include "log_repository.h"


class LogService {
    public:
        LogService(LogRepository& repo);

    private:
        LogRepository& m_repo;

};


