#pragma once

#include <string>
#include "stinger/utils/mqttproperties.hpp"

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

    MqttMessage()
        : topic(""),
          payload(""),
          qos(0),
          retain(false),
          properties()
    {}

    MqttMessage(const std::string& topic, 
                const std::string& payload,
                unsigned qos = 0,
                bool retain = false,
                const MqttProperties& props = MqttProperties())
        : topic(topic),
          payload(payload),
          qos(qos),
          retain(retain),
          properties(props)
    {}
};

} // namespace utils
} // namespace stinger
