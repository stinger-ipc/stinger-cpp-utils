#include "stinger/mqtt/message.hpp"
#include "stinger/utils/conversions.hpp"

namespace stinger {
namespace mqtt {

Message::Message(const std::string& topic, const std::string& payload, unsigned qos, bool retain,
                 const Properties& props)
    : topic(topic), payload(utils::toBytes(payload)), qos(qos), retain(retain), properties(props) {}

Message::Message(const Message& other)
    : topic(other.topic), payload(other.payload), qos(other.qos), retain(other.retain), properties(other.properties) {}

Message Message::Signal(const std::string& topic, const std::string& payload) {
    Properties props;
    props.contentType = "application/json";
    return Message(topic, payload, 2, false, props);
}

Message Message::PropertyValue(const std::string& topic, const std::string& payload, int propertyVersion) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    return Message(topic, payload, 1, true, props);
}

Message Message::PropertyUpdateRequest(const std::string& topic, const std::string& payload, int propertyVersion,
                                       const std::vector<std::byte>& correlationData,
                                       const std::string& responseTopic) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.responseTopic = responseTopic;
    return Message(topic, payload, 1, false, props);
}

Message Message::PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
                                        const std::optional<std::vector<std::byte>>& correlationData,
                                        stinger::error::MethodReturnCode returnCode, const std::string& debugMessage) {
    Properties props;
    props.contentType = "application/json";
    props.propertyVersion = propertyVersion;
    props.correlationData = correlationData;
    props.returnCode = static_cast<int>(returnCode);
    props.debugInfo = debugMessage;
    return Message(topic, payload, 1, false, props);
}

Message Message::PropertyUpdateResponse(const std::string& topic, const std::string& payload, int propertyVersion,
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
                               const std::vector<std::byte>& correlationData, const std::string& responseTopic) {
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

Message Message::ServiceOnline(const std::string& topic, const std::string& payload, int messageExpiryInterval) {
    Properties props;
    props.contentType = "application/json";
    props.messageExpiryInterval = messageExpiryInterval;
    return Message(topic, payload, 1, true, props);
}

Message Message::ServiceOffline(const std::string& topic) {
    return Message(topic, "", 1, true, Properties());
}

BinaryMessage::BinaryMessage(const std::string& topic, const std::vector<std::byte>& payload,
                             const std::string& contentType, unsigned qos, bool retain)
    : Message(topic, "", qos, retain) {
    this->payload = payload;
    this->properties.contentType = contentType;
}

BinaryMessage BinaryMessage::BinarySignal(const std::string& topic, const std::vector<std::byte>& payload,
                                          const std::string& contentType) {
    return BinaryMessage(topic, payload, contentType, 2, false);
}

BinaryMessage BinaryMessage::BinaryPropertyValue(const std::string& topic, const std::vector<std::byte>& payload,
                                                 const std::string& contentType, int propertyVersion) {
    BinaryMessage msg(topic, payload, contentType, 1, true);
    msg.properties.propertyVersion = propertyVersion;
    return msg;
}

BinaryMessage BinaryMessage::BinaryPropertyUpdateRequest(const std::string& topic,
                                                         const std::vector<std::byte>& payload,
                                                         const std::string& contentType, int propertyVersion,
                                                         const std::vector<std::byte>& correlationData,
                                                         const std::string& responseTopic) {
    BinaryMessage msg(topic, payload, contentType, 1, false);
    msg.properties.propertyVersion = propertyVersion;
    msg.properties.correlationData = correlationData;
    msg.properties.responseTopic = responseTopic;
    return msg;
}

BinaryMessage BinaryMessage::BinaryPropertyUpdateResponse(const std::string& topic,
                                                          const std::vector<std::byte>& payload,
                                                          const std::string& contentType, int propertyVersion,
                                                          const std::optional<std::vector<std::byte>>& correlationData,
                                                          stinger::error::MethodReturnCode returnCode,
                                                          const std::string& debugMessage) {
    BinaryMessage msg(topic, payload, contentType, 1, false);
    msg.properties.propertyVersion = propertyVersion;
    msg.properties.correlationData = correlationData;
    msg.properties.returnCode = static_cast<int>(returnCode);
    msg.properties.debugInfo = debugMessage;
    return msg;
}

BinaryMessage BinaryMessage::BinaryPropertyUpdateResponse(const std::string& topic,
                                                          const std::vector<std::byte>& payload,
                                                          const std::string& contentType, int propertyVersion,
                                                          const std::optional<std::vector<std::byte>>& correlationData,
                                                          stinger::error::MethodReturnCode returnCode) {
    BinaryMessage msg(topic, payload, contentType, 1, false);
    msg.properties.propertyVersion = propertyVersion;
    msg.properties.correlationData = correlationData;
    msg.properties.returnCode = static_cast<int>(returnCode);
    return msg;
}

BinaryMessage BinaryMessage::BinaryMethodRequest(const std::string& topic, const std::vector<std::byte>& payload,
                                                 const std::string& contentType,
                                                 const std::vector<std::byte>& correlationData,
                                                 const std::string& responseTopic) {
    BinaryMessage msg(topic, payload, contentType, 2, false);
    msg.properties.correlationData = correlationData;
    msg.properties.responseTopic = responseTopic;
    return msg;
}

BinaryMessage BinaryMessage::BinaryMethodResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                                  const std::string& contentType,
                                                  const std::optional<std::vector<std::byte>>& correlationData,
                                                  stinger::error::MethodReturnCode returnCode,
                                                  const std::string& debugMessage) {
    BinaryMessage msg(topic, payload, contentType, 1, false);
    msg.properties.correlationData = correlationData;
    msg.properties.returnCode = static_cast<int>(returnCode);
    msg.properties.debugInfo = debugMessage;
    return msg;
}

BinaryMessage BinaryMessage::BinaryMethodResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                                  const std::string& contentType,
                                                  const std::optional<std::vector<std::byte>>& correlationData,
                                                  stinger::error::MethodReturnCode returnCode) {
    BinaryMessage msg(topic, payload, contentType, 1, false);
    msg.properties.correlationData = correlationData;
    msg.properties.returnCode = static_cast<int>(returnCode);
    return msg;
}

BinaryMessage BinaryMessage::BinaryServiceOnline(const std::string& topic, const std::vector<std::byte>& payload,
                                                 const std::string& contentType, int messageExpiryInterval) {
    BinaryMessage msg(topic, payload, contentType, 1, true);
    msg.properties.messageExpiryInterval = messageExpiryInterval;
    return msg;
}

} // namespace mqtt
} // namespace stinger
