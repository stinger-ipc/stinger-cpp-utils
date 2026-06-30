#pragma once

#include "stinger/error/return_codes.hpp"
#include "stinger/mqtt/properties.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace stinger {
namespace mqtt {

/**
 * @brief Represents an MQTT message
 */
struct Message {
    std::string topic;
    std::vector<std::byte> payload;
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
                                          stinger::error::MethodReturnCode returnCode, const std::string& debugMessage);

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

    virtual ~Message() = default;
};

/**
 * @brief An MQTT message that carries binary (non-text) data.
 *
 * Unlike Message, whose factory methods accept text payloads, BinaryMessage is
 * intended for arbitrary binary content and requires a content-type which is
 * stored in the message properties.
 */
class BinaryMessage : public Message {
public:
    BinaryMessage(const std::string& topic, const std::vector<std::byte>& payload, const std::string& contentType,
                  unsigned qos = 0, bool retain = false);

    static BinaryMessage BinarySignal(const std::string& topic, const std::vector<std::byte>& payload,
                                      const std::string& contentType);

    static BinaryMessage BinaryPropertyValue(const std::string& topic, const std::vector<std::byte>& payload,
                                             const std::string& contentType, int propertyVersion);

    static BinaryMessage BinaryPropertyUpdateRequest(const std::string& topic, const std::vector<std::byte>& payload,
                                                     const std::string& contentType, int propertyVersion,
                                                     const std::vector<std::byte>& correlationData,
                                                     const std::string& responseTopic);

    static BinaryMessage BinaryPropertyUpdateResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                                      const std::string& contentType, int propertyVersion,
                                                      const std::optional<std::vector<std::byte>>& correlationData,
                                                      stinger::error::MethodReturnCode returnCode,
                                                      const std::string& debugMessage);

    static BinaryMessage BinaryPropertyUpdateResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                                      const std::string& contentType, int propertyVersion,
                                                      const std::optional<std::vector<std::byte>>& correlationData,
                                                      stinger::error::MethodReturnCode returnCode);

    static BinaryMessage BinaryMethodRequest(const std::string& topic, const std::vector<std::byte>& payload,
                                             const std::string& contentType,
                                             const std::vector<std::byte>& correlationData,
                                             const std::string& responseTopic);

    static BinaryMessage BinaryMethodResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                              const std::string& contentType,
                                              const std::optional<std::vector<std::byte>>& correlationData,
                                              stinger::error::MethodReturnCode returnCode,
                                              const std::string& debugMessage);

    static BinaryMessage BinaryMethodResponse(const std::string& topic, const std::vector<std::byte>& payload,
                                              const std::string& contentType,
                                              const std::optional<std::vector<std::byte>>& correlationData,
                                              stinger::error::MethodReturnCode returnCode);

    static BinaryMessage BinaryServiceOnline(const std::string& topic, const std::vector<std::byte>& payload,
                                             const std::string& contentType, int messageExpiryInterval);
};

} // namespace mqtt
} // namespace stinger
