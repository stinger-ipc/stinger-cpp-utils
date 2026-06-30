#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace stinger {
namespace utils {

// Utility function to convert ISO timestamp string to time_point
std::chrono::time_point<std::chrono::system_clock> parseIsoTimestamp(const std::string& isoTimestamp);

// Utility function to convert time_point to ISO timestamp string
std::string timePointToIsoString(const std::chrono::time_point<std::chrono::system_clock>& timePoint);

std::string durationToIsoString(const std::chrono::duration<double>& duration);

std::chrono::duration<double> parseIsoDuration(const std::string& isoDuration);

// Base64 encode binary data; takes a vector of unsigned bytes and returns std::string
std::string base64Encode(const std::vector<unsigned char>& data);

// Decode a base64 encoded string into a vector of bytes
std::vector<unsigned char> base64Decode(const std::string& b64input);

// Convert a string into a binary payload.
std::vector<std::byte> toBytes(const std::string& str);

// Convert a binary payload into a string.
std::string toString(const std::vector<std::byte>& bytes);

} // namespace utils
} // namespace stinger
