#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace stinger {
namespace utils {

/**
 * Compute a fast, non-cryptographic hash of a single string.
 * Uses FNV-1a algorithm for speed and good distribution.
 *
 * @param str String to hash
 * @return 64-bit hash value suitable for use as a map key
 */
uint64_t hashString(const std::string& str);

/**
 * Compute a fast, non-cryptographic hash of a vector of strings.
 * Uses FNV-1a algorithm for speed and good distribution.
 *
 * @param strings Vector of strings to hash
 * @return 64-bit hash value suitable for use as a map key
 */
uint64_t hashStrings(const std::vector<std::string>& strings);

} // namespace utils
} // namespace stinger
