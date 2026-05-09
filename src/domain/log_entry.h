
#pragma once


#include <string>
#include <optional>


struct LogEntry {
    int id = 0;
    std::string message;
    std::string level;
    std::string source;
    std::string timestamp;
};

struct FilterParams {
    std::optional<std::string> level;
    std::optional<std::string> source;
    std::optional<std::string> since;
    std::optional<std::string> until;
    std::optional<int> limit;
};

