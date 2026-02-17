#include "stinger/mqtt/message.hpp"

namespace stinger {
namespace mqtt {

Message::Message(const std::string& topic, const std::string& payload, unsigned qos, bool retain,
                         const Properties& props)
    : topic(topic), payload(payload), qos(qos), retain(retain), properties(props) {}

Message::Message(const Message& other)
    : topic(other.topic), payload(other.payload), qos(other.qos), retain(other.retain), properties(other.properties) {}

Message Message::Signal(const std::string& topic, const std::string& payload) {
    Properties props;
    props.contentType = "application/json";
    return Message(topic, payload, 2, false, Properties());
}

Message Message::PropertyValue(const std::string& topic, const std::string& payload, int propertyVersion) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    return Message(topic, payload, 1, true, props);
}

Message Message::PropertyUpdateRequest(const std::string& topic, const std::string& payload,
                                               int propertyVersion, const std::vector<std::byte>& correlationData,
                                               const std::string& responseTopic) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.responseTopic = responseTopic;
    return Message(topic, payload, 1, false, props);
}

Message Message::PropertyUpdateResponse(const std::string& topic, const std::string& payload,
                                                int propertyVersion,
                                                const std::optional<std::vector<std::byte>>& correlationData,
                                                stinger::error::MethodReturnCode returnCode,
                                                const std::string& debugMessage) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    props.debugInfo = debugMessage;
    return Message(topic, payload, 1, false, props);
}

Message Message::PropertyUpdateResponse(const std::string& topic, const std::string& payload,
                                                int propertyVersion,
                                                const std::optional<std::vector<std::byte>>& correlationData,
                                                stinger::error::MethodReturnCode returnCode) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    return Message(topic, payload, 1, false, props);
}

Message Message::MethodRequest(const std::string& topic, const std::string& payload,
                                       const std::vector<std::byte>& correlationData,
                                       const std::string& responseTopic) {
    Properties props;
    props.contentType = "application/json";
    props.correlationData = correlationData;
    props.responseTopic = responseTopic;
    return Message(topic, payload, 2, false, props);
}

Message Message::MethodResponse(const std::string& topic, const std::string& payload,
                                        const std::optional<std::vector<std::byte>>& correlationData,
                                        stinger::error::MethodReturnCode returnCode, const std::string& debugMessage) {
    Properties props;
    props.contentType = "application/json";
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    props.debugInfo = debugMessage;
    return Message(topic, payload, 1, false, props);
}

Message Message::MethodResponse(const std::string& topic, const std::string& payload,
                                        const std::optional<std::vector<std::byte>>& correlationData,
                                        stinger::error::MethodReturnCode returnCode) {
    Properties props;
    props.contentType = "application/json";
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    return Message(topic, payload, 1, false, props);
}

Message Message::ServiceOnline(const std::string& topic, const std::string& payload,
                                       int messageExpiryInterval) {
    Properties props;
    props.contentType = "application/json";
    props.messageExpiryInterval = messageExpiryInterval;
    return Message(topic, payload, 1, true, props);
}

Message Message::ServiceOffline(const std::string& topic) {
    return Message(topic, "", 1, true, Properties());
}

} // namespace mqtt
} // namespace stinger
