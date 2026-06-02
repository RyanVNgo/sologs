
#include "crypto.h"

#include <iomanip>
#include <openssl/evp.h>
#include <sstream>
#include <array>
#include <fstream>
#include <sys/random.h>


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace sologs::crypto {

std::string sha256_hex(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

std::string generate_uuid() {
    return boost::uuids::to_string(boost::uuids::random_generator{}());
}

std::string generate_key() {
    std::array<uint8_t, 32> bytes;
    if (getrandom(bytes.data(), bytes.size(), 0) != bytes.size()) {
        std::ifstream urandom("/dev/urandom", std::ios::binary);
        urandom.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    }

    static const char* tbl = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string out;
    out.reserve(44);

    uint32_t buffer = 0;
    int bits = 0;
    for (uint8_t b : bytes) {
        buffer = (buffer << 8) | b;
        bits += 8;

        while (bits >= 6) {
            out.push_back(tbl[(buffer >> (bits - 6)) & 0x3F]);
            bits -= 6;
        }
    }

    if (bits > 0) {
        out.push_back(tbl[(buffer << (6 - bits)) & 0x3F]);
    }

    return out;
}

} // sologs::crypto

