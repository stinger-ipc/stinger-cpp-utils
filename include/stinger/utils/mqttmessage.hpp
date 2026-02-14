#pragma once

#include "stinger/error/return_codes.hpp"
#include "stinger/utils/mqttproperties.hpp"
#include <string>

namespace stinger {
namespace utils {

/**
 * @brief Represents an MQTT message
 */
struct MqttMessage {
    std::string topic;
    std::string payload;
    unsigned qos;
    bool retain;
    MqttProperties properties;

    MqttMessage(const std::string& topic, const std::string& payload, unsigned qos = 0, bool retain = false,
                const MqttProperties& props = MqttProperties());

    MqttMessage(const MqttMessage& other);

    static MqttMessage Signal(const std::string& topic, const std::string& payload);

    static MqttMessage PropertyValue(const std::string& topic, const std::string& payload, int propertyVersion);

    static MqttMessage PropertyUpdateRequest(const std::string& topic, const std::string& payload, int propertyVersion,
                                             const std::vector<std::byte>& correlationData,
                                             const std::string& responseTopic);

    static MqttMessage PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
                                              const std::vector<std::byte>& correlationData,
                                              stinger::error::MethodReturnCode returnCode,
                                              const std::string& debugMessage);

    static MqttMessage PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
                                              const std::vector<std::byte>& correlationData,
                                              stinger::error::MethodReturnCode returnCode);

    static MqttMessage MethodRequest(const std::string& topic, const std::string& payload,
                                     const std::vector<std::byte>& correlationData, const std::string& responseTopic);
                                     
    static MqttMessage MethodResponse(const std::string& topic, const std::string& payload,
                                      const std::vector<std::byte>& correlationData,
                                      stinger::error::MethodReturnCode returnCode, const std::string& debugMessage);

    static MqttMessage MethodResponse(const std::string& topic, const std::string& payload,
                                      const std::vector<std::byte>& correlationData,
                                      stinger::error::MethodReturnCode returnCode);
};

} // namespace utils
} // namespace stinger
