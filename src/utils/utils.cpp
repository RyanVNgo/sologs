
#include "utils.h"

#include <sstream>


namespace sologs::utils {

auto permission_label(Permissions perm) -> std::string {
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
) -> std::optional<Permissions> {
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
) -> std::string {
    std::ostringstream oss;
    for (size_t i = 0; i < perms.size(); ++i) {
        if (i > 0) oss << ",";
        oss << permission_label(perms[i]);
    }
    return oss.str();
}

auto parse_permissions(const std::string& str) -> std::vector<Permissions> {
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

} // sologs::utils

