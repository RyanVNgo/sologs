
#include "crypto.h"

#include <iomanip>
#include <openssl/evp.h>
#include <sstream>
#include <array>
#include <fstream>
#include <sys/random.h>

#include <uuid.h>

namespace sologs::crypto {

auto sha256_hex(const std::string& input) -> std::string {
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

auto generate_uuid() -> std::string {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};
    uuids::uuid const id = gen();
    return uuids::to_string(id);
}

auto generate_key() -> std::string {
    std::array<uint8_t, 32> bytes;
    if (getrandom(bytes.data(), bytes.size(), 0) != static_cast<long int>(bytes.size())) {
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

