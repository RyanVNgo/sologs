
#pragma once

#include <string>


namespace sologs::crypto {

    std::string sha256_hex(const std::string& input);

    std::string generate_uuid();

    std::string generate_key();

} // sologs::crypto

