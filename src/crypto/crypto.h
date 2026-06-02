
#pragma once

#include <string>


namespace sologs::crypto {

    auto sha256_hex(const std::string& input) -> std::string;

    auto generate_uuid() -> std::string;

    auto generate_key() -> std::string;

} // sologs::crypto

