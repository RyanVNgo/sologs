
#pragma once

#include <string>


namespace sologs::crypto {

    [[nodiscard]] auto sha256_hex(const std::string& input) -> std::string;

    [[nodiscard]] auto generate_uuid() -> std::string;

    [[nodiscard]] auto generate_key() -> std::string;

} // sologs::crypto

