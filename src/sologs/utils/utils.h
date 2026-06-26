
#pragma once

#include <domain/auth.h>

#include <optional>


namespace sologs::utils {

[[nodiscard]]
auto permission_label(
        Permissions perm
) noexcept -> std::string;

[[nodiscard]]
auto permission_from_label(
        const std::string& label
) noexcept -> std::optional<Permissions>;

[[nodiscard]]
auto permissions_to_string(
        const std::vector<Permissions>& perms
) noexcept -> std::string;

[[nodiscard]]
auto parse_permissions(
        const std::string& str
) noexcept -> std::vector<Permissions>;

[[nodiscard]]
auto is_valid_datetime(
        const std::string& str
) noexcept -> bool;

} // sologs::utils

