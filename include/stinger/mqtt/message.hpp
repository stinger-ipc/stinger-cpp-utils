#pragma once

#include "stinger/error/return_codes.hpp"
#include "stinger/mqtt/properties.hpp"
#include <optional>
#include <string>

namespace stinger {
namespace mqtt {

/**
 * @brief Represents an MQTT message
 */
struct Message {
    std::string topic;
    std::string payload;
    unsigned qos;
    bool retain;
    Properties properties;

    Message(const std::string& topic, const std::string& payload, unsigned qos = 0, bool retain = false,
                const Properties& props = Properties());

    Message(const Message& other);

    static Message Signal(const std::string& topic, const std::string& payload);

    static Message PropertyValue(const std::string& topic, const std::string& payload, int propertyVersion);

    static Message PropertyUpdateRequest(const std::string& topic, const std::string& payload, int propertyVersion,
                                             const std::vector<std::byte>& correlationData,
                                             const std::string& responseTopic);

    static Message PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
                                              const std::optional<std::vector<std::byte>>& correlationData,
                                              stinger::error::MethodReturnCode returnCode,
                                              const std::string& debugMessage);

    static Message PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
                                              const std::optional<std::vector<std::byte>>& correlationData,
                                              stinger::error::MethodReturnCode returnCode);

    static Message MethodRequest(const std::string& topic, const std::string& payload,
                                     const std::vector<std::byte>& correlationData, const std::string& responseTopic);

    static Message MethodResponse(const std::string& topic, const std::string& payload,
                                      const std::optional<std::vector<std::byte>>& correlationData,
                                      stinger::error::MethodReturnCode returnCode, const std::string& debugMessage);

    static Message MethodResponse(const std::string& topic, const std::string& payload,
                                      const std::optional<std::vector<std::byte>>& correlationData,
                                      stinger::error::MethodReturnCode returnCode);

    static Message ServiceOnline(const std::string& topic, const std::string& payload, int messageExpiryInterval);

    static Message ServiceOffline(const std::string& topic);
};

} // namespace mqtt
} // namespace stinger
