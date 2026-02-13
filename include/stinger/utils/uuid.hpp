
#pragma once

#include <iomanip>
#include <random>
#include <sstream>
#include <string>

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
