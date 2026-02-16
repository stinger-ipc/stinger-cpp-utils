#include "stinger/utils/mqttmessage.hpp"

namespace stinger {
namespace utils {

MqttMessage::MqttMessage(const std::string& topic, const std::string& payload, unsigned qos, bool retain,
                         const MqttProperties& props)
    : topic(topic), payload(payload), qos(qos), retain(retain), properties(props) {}

MqttMessage::MqttMessage(const MqttMessage& other)
    : topic(other.topic), payload(other.payload), qos(other.qos), retain(other.retain), properties(other.properties) {}

MqttMessage MqttMessage::Signal(const std::string& topic, const std::string& payload) {
    return MqttMessage(topic, payload, 2, false, MqttProperties());
}

MqttMessage MqttMessage::PropertyValue(const std::string& topic, const std::string& payload, int propertyVersion) {
    MqttProperties props;
    props.propertyVersion = propertyVersion;
    return MqttMessage(topic, payload, 1, true, props);
}

MqttMessage MqttMessage::PropertyUpdateRequest(const std::string& topic, const std::string& payload,
                                               int propertyVersion, const std::vector<std::byte>& correlationData,
                                               const std::string& responseTopic) {
    MqttProperties props;
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.responseTopic = responseTopic;
    return MqttMessage(topic, payload, 1, false, props);
}

MqttMessage MqttMessage::PropertyUpdateResponse(const std::string& topic, const std::string& payload,
                                                int propertyVersion, const std::vector<std::byte>& correlationData,
                                                stinger::error::MethodReturnCode returnCode,
                                                const std::string& debugMessage) {
    MqttProperties props;
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    props.debugInfo = debugMessage;
    return MqttMessage(topic, payload, 1, false, props);
}

MqttMessage MqttMessage::PropertyUpdateResponse(const std::string& topic, const std::string& payload,
                                                int propertyVersion, const std::vector<std::byte>& correlationData,
                                                stinger::error::MethodReturnCode returnCode) {
    MqttProperties props;
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    return MqttMessage(topic, payload, 1, false, props);
}

MqttMessage MqttMessage::MethodRequest(const std::string& topic, const std::string& payload,
                                       const std::vector<std::byte>& correlationData,
                                       const std::string& responseTopic) {
    MqttProperties props;
    props.correlationData = correlationData;
    props.responseTopic = responseTopic;
    return MqttMessage(topic, payload, 2, false, props);
}

MqttMessage MqttMessage::MethodResponse(const std::string& topic, const std::string& payload,
                                        const std::vector<std::byte>& correlationData,
                                        stinger::error::MethodReturnCode returnCode, const std::string& debugMessage) {
    MqttProperties props;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    props.debugInfo = debugMessage;
    return MqttMessage(topic, payload, 1, false, props);
}

MqttMessage MqttMessage::MethodResponse(const std::string& topic, const std::string& payload,
                                        const std::vector<std::byte>& correlationData,
                                        stinger::error::MethodReturnCode returnCode) {
    MqttProperties props;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    return MqttMessage(topic, payload, 1, false, props);
}

MqttMessage MqttMessage::ServiceOnline(const std::string& topic, const std::string& payload,
                                       int messageExpiryInterval) {
    MqttProperties props;
    props.messageExpiryInterval = messageExpiryInterval;
    return MqttMessage(topic, payload, 1, true, props);
}

MqttMessage MqttMessage::ServiceOffline(const std::string& topic) {
    return MqttMessage(topic, "", 1, true, MqttProperties());
}

} // namespace utils
} // namespace stinger
