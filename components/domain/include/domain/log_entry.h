
#pragma once

#include <string>
#include <optional>


struct LogEntry {
    int id = 0;
    std::string message{};
    std::string level{};
    std::string source{};
    std::string timestamp{};
};

struct LogFilterParams {
    static constexpr const char* LevelKey = "level";
    static constexpr const char* SourceKey = "source";
    static constexpr const char* SinceKey = "since";
    static constexpr const char* UntilKey = "until";
    static constexpr const char* LimitKey = "limit";

    std::optional<std::string>  level{std::nullopt};
    std::optional<std::string>  source{std::nullopt};
    std::optional<std::string>  since{std::nullopt};
    std::optional<std::string>  until{std::nullopt};
    std::optional<int>          limit{std::nullopt};
};


