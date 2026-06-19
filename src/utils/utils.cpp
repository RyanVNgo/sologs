
#include "utils.h"

#include <sstream>


namespace sologs::utils {

auto permission_label(
        Permissions perm
) noexcept -> std::string {
    switch (perm) {
        case Permissions::LogRead:    return "LogRead";
        case Permissions::LogWrite:   return "LogWrite";
        case Permissions::LogDelete:  return "LogDelete";
        case Permissions::AuthRead:   return "AuthRead";
        case Permissions::AuthWrite:  return "AuthWrite";
        case Permissions::AuthDelete: return "AuthDelete";
        case Permissions::Admin:      return "Admin";
    }
    return "";
}

auto permission_from_label(
        const std::string& label
) noexcept -> std::optional<Permissions> {
    if (label == "LogRead")    return Permissions::LogRead;
    if (label == "LogWrite")   return Permissions::LogWrite;
    if (label == "LogDelete")  return Permissions::LogDelete;
    if (label == "AuthRead")   return Permissions::AuthRead;
    if (label == "AuthWrite")  return Permissions::AuthWrite;
    if (label == "AuthDelete") return Permissions::AuthDelete;
    if (label == "Admin")      return Permissions::Admin;
    return {};
}

auto permissions_to_string(
        const std::vector<Permissions>& perms
) noexcept -> std::string {
    std::ostringstream oss;
    for (size_t i = 0; i < perms.size(); ++i) {
        if (i > 0) oss << ",";
        oss << permission_label(perms[i]);
    }
    return oss.str();
}

auto parse_permissions(
        const std::string& str
) noexcept -> std::vector<Permissions> {
    std::vector<Permissions> result;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (token.empty()) continue;
        auto perm = permission_from_label(token);
        if (perm.has_value()) {
            result.push_back(perm.value());
        }
    }
    return result;
}

auto is_valid_datetime(
        const std::string& str
) noexcept -> bool {
    if (str.size() < 19) {
        return false;
    }

    auto is_digit = [](char c) {
        return c >= '0' && c <= '9';
    };

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            if (str[i] != '-') {
                return false;
            }
        } else {
            if (!is_digit(str[i])) {
                return false;
            }
        }
    }

    if (str[10] != 'T' && str[10] != ' ') {
        return false;
    }

    for (int i = 11; i < 19; ++i) {
        if (i == 13 || i == 16) {
            if (str[i] != ':') {
                return false;
            }
        } else {
            if (!is_digit(str[i])) {
                return false;
            }
        }
    }

    size_t pos = 19;
    if (pos < str.size() && str[pos] == 'Z') {
        pos++;
    }
    if (pos != str.size()) {
        return false;
    }

    int year  = std::stoi(str.substr(0, 4));
    int month = std::stoi(str.substr(5, 2));
    int day   = std::stoi(str.substr(8, 2));
    int hour  = std::stoi(str.substr(11, 2));
    int min   = std::stoi(str.substr(14, 2));
    int sec   = std::stoi(str.substr(17, 2));

    if (month < 1 || month > 12) {
        return false;
    }
    if (hour < 0 || hour > 23) {
        return false;
    }
    if (min < 0 || min > 59) {
        return false;
    }
    if (sec < 0 || sec > 59) {
        return false;
    }

    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int max = days[month - 1];
    if (month == 2 && (year % 400 == 0 || (year % 100 != 0 && year % 4 == 0))) {
        max = 29;
    }
    if (day < 1 || day > max) {
        return false;
    }

    return true;
}

} // sologs::utils

