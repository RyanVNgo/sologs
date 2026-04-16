
#pragma once


#include "json.hpp"

#include "log_repository.h"


using json = nlohmann::json;

class LogService {
    public:
        LogService(LogRepository& repo);

        bool create_log(const json& body);
        json get_logs();

    private:
        LogRepository& m_repo;

};


