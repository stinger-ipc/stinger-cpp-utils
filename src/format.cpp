#include "stinger/utils/format.hpp"

namespace stinger {
namespace utils {

std::string format(const std::string& templateStr, const std::map<std::string, std::string>& values) {
    std::string result = templateStr;
    
    // Iterate through all key-value pairs in the map
    for (const auto& [key, value] : values) {
        std::string token = "{" + key + "}";
        size_t pos = 0;
        
        // Replace all occurrences of the token in the string
        while ((pos = result.find(token, pos)) != std::string::npos) {
            result.replace(pos, token.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

} // namespace utils
} // namespace stinger
