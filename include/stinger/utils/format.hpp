#pragma once

#include <map>
#include <string>

namespace stinger {
namespace utils {

/**
 * @brief Replaces {token} placeholders in a template string with values from a map
 *
 * @param templateStr The template string containing {token} placeholders
 * @param values A map of token names to their replacement values
 * @return std::string The formatted string with all tokens replaced
 *
 * @example
 * std::map<std::string, std::string> values = {
 *     {"name", "World"},
 *     {"greeting", "Hello"}
 * };
 * std::string result = format("{greeting}, {name}!", values);
 * // result == "Hello, World!"
 */
std::string format(const std::string& templateStr, const std::map<std::string, std::string>& values);

} // namespace utils
} // namespace stinger
