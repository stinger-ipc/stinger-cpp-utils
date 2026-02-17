#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace stinger {
namespace mqtt {

struct Properties {
    Properties()
        : correlationData(std::nullopt), responseTopic(std::nullopt), subscriptionId(std::nullopt),
          messageExpiryInterval(std::nullopt), contentType(std::nullopt), debugInfo(std::nullopt),
          returnCode(std::nullopt), propertyVersion(std::nullopt), version(std::nullopt) {}
    std::optional<std::vector<std::byte>> correlationData;
    std::optional<std::string> responseTopic;
    std::optional<std::uint32_t> subscriptionId; // Ignored on publish
    std::optional<std::uint32_t> messageExpiryInterval;
    std::optional<std::string> contentType;
    // The following are User Properties.
    std::optional<std::string> debugInfo; // Used to pass a human readable debug message back to the client.
    std::optional<int> returnCode;        // Used to pass a numeric method return code back to the client.
    std::optional<int> propertyVersion;   // Used to specify the modification count of a property.
    std::optional<std::string> version;   // Used to specify the version of the method, property, or signal.
};

} // namespace mqtt
} // namespace stinger