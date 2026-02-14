
#pragma once

#include <cstddef>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace stinger {
namespace utils {

inline std::vector<std::byte> generate_uuid_bytes() {
    static std::random_device rd;
    static std::mt19937_64 eng(rd());
    static std::uniform_int_distribution<uint64_t> dist;

    std::vector<std::byte> uuid(16);
    uint64_t part1 = dist(eng);
    uint64_t part2 = dist(eng);

    // Copy first 8 bytes from part1
    for (int i = 0; i < 8; ++i) {
        uuid[i] = static_cast<std::byte>((part1 >> (i * 8)) & 0xFF);
    }

    // Copy next 8 bytes from part2
    for (int i = 0; i < 8; ++i) {
        uuid[8 + i] = static_cast<std::byte>((part2 >> (i * 8)) & 0xFF);
    }

    return uuid;
}

inline std::string generate_uuid_string() {
    static std::random_device rd;
    static std::mt19937_64 eng(rd());
    static std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(16) << dist(eng);
    ss << std::setw(16) << dist(eng);
    return ss.str();
}

} // namespace utils
} // namespace stinger