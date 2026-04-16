
#pragma once


#include <string>


struct LogEntry {
    int id = 0;
    std::string message;
    std::string level;
    std::string source;
    std::string timestamp;
};

