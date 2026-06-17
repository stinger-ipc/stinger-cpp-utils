
#pragma once
#include <future>

#include "stinger/mqtt/message.hpp"

namespace stinger {
namespace utils {

/**
 * @brief Container and JSON utility functions
 */

typedef std::function<void(int, const char*)> LogFunctionType;
typedef int CallbackHandleType;

class IConnection {
public:
    /*! Publish a message.
     * Implementations should queue up messages when not connected.
     * A future is returned which resolves when the message is received by the broker, depending on QoS.
     */
    virtual std::future<bool> Publish(const stinger::mqtt::Message& mqttMsg) = 0;

    /*! Subscribe to a topic.
     * Implementation should queue up subscriptions when not connected.
     * Returns a subscription identifier.
     */
    virtual int Subscribe(const std::string& topic, int qos) = 0;

    virtual void Unsubscribe(const std::string& topic) = 0;

    /*! Provide a callback to be called on an incoming message.
     * Implementation should accept this at any time, even when not connected.
     */
    virtual CallbackHandleType AddMessageCallback(const std::function<void(const stinger::mqtt::Message&)>& cb) = 0;

    virtual void RemoveMessageCallback(CallbackHandleType handle) = 0;

    /*! Utility for matching topics.
     * This probably should be a wrapper around `mosquitto_topic_matches_sub` or similar
     */
    virtual bool TopicMatchesSubscription(const std::string& topic, const std::string& subscr) const = 0;

    /*!
     * Get the client ID of the connection.
     */
    virtual std::string GetClientId() const = 0;

    /*!
     * Get the topic for the last will message.
     */
    virtual std::string GetLastWillTopic() const = 0;

    /*!
     * Get the topic for the online message.
     * @deprecated Use GetLastWillTopic instead, as the online message is published to the last will topic.
     */
    virtual std::string GetOnlineTopic() const { return GetLastWillTopic(); }

    /*!
     * Get the payload for the online message.  This will be automatically published to the Last Will topic when
     * connected and periodically.
     */
    virtual std::string GetOnlinePayload() const = 0;

    /*!
     * Get the payload for the offline message.  This will be used as the last will message, and will be published (by
     * the broker) to the Last Will topic if the client disconnects unexpectedly.
     */
    virtual std::string GetOfflinePayload() const = 0;

    /*!
     * Log a message with the given level and format.
     */
    virtual void Log(int level, const char* fmt, ...) const = 0;
};

} // namespace utils
} // namespace stinger
