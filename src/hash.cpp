#include <stinger/utils/hash.hpp>

namespace stinger {
namespace utils {

uint64_t hashString(const std::string& str) {
    constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET_BASIS;
    for (unsigned char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_PRIME;
    }
    return hash;
}

uint64_t hashStrings(const std::vector<std::string>& strings) {
    // FNV-1a hash constants for 64-bit
    constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET_BASIS;

    for (const auto& str : strings) {
        for (unsigned char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }
        // Mix in a separator between strings to ensure ["ab", "c"] != ["a", "bc"]
        hash ^= 0xFF;
        hash *= FNV_PRIME;
    }

    return hash;
}

} // namespace utils
} // namespace stinger
