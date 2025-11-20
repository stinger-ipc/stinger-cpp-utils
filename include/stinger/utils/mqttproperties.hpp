#pragma once

#include <optional>
#include <string>

namespace stinger {
namespace utils {

struct MqttProperties
{
    MqttProperties()
        : correlationId(std::nullopt),
            responseTopic(std::nullopt),
            returnCode(std::nullopt),
            subscriptionId(std::nullopt),
            propertyVersion(std::nullopt),
            messageExpiryInterval(std::nullopt),
            debugInfo(std::nullopt)
    {}
    std::optional<std::string> correlationId;
    std::optional<std::string> responseTopic;
    std::optional<int> returnCode;
    std::optional<int> subscriptionId;
    std::optional<int> propertyVersion;
    std::optional<int> messageExpiryInterval;
    std::optional<std::string> debugInfo;
};

} // namespace utils
} // namespace stinger