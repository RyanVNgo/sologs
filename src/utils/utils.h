
#pragma once

#include "auth.h"

#include <optional>


namespace sologs::utils {

auto permission_label(Permissions perm) -> std::string;

auto permission_from_label(
        const std::string& label
) -> std::optional<Permissions>;

auto permissions_to_string(
        const std::vector<Permissions>& perms
) -> std::string;

auto parse_permissions(const std::string& str) -> std::vector<Permissions>;

} // sologs::utils

