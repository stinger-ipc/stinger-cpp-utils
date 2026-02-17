#pragma once

#include "stinger/utils/iconnection.hpp"
#include "stinger/mqtt/message.hpp"
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace stinger {
namespace utils {

/**
 * @brief Mock MQTT connection for unit testing
 *
 * This class implements IConnection without requiring an actual MQTT broker.
 * It allows testing of code that uses MQTT connections by providing:
 * - Message queuing and retrieval
 * - Subscription tracking
 * - Callback simulation
 */
class MockConnection : public IConnection {
public:
    MockConnection(const std::string& clientId);
    virtual ~MockConnection();

    // IConnection interface implementation
    virtual std::future<bool> Publish(const stinger::mqtt::Message& mqttMsg) override;
    virtual int Subscribe(const std::string& topic, int qos) override;
    virtual void Unsubscribe(const std::string& topic) override;
    virtual CallbackHandleType AddMessageCallback(const std::function<void(const stinger::mqtt::Message&)>& cb) override;
    virtual void RemoveMessageCallback(CallbackHandleType handle) override;
    virtual bool TopicMatchesSubscription(const std::string& topic, const std::string& subscr) const override;
    virtual std::string GetClientId() const override;
    virtual std::string GetOnlineTopic() const override;
    virtual void Log(int level, const char* fmt, ...) const override;

    // Testing utility methods

    /*! Simulate receiving a message - triggers callbacks for matching subscriptions */
    void SimulateIncomingMessage(const stinger::mqtt::Message& msg);

    /*! Get all published messages */
    std::vector<stinger::mqtt::Message> GetPublishedMessages() const;

    /*! Get published messages for a specific topic */
    std::vector<stinger::mqtt::Message> GetPublishedMessages(const std::string& topic) const;

    /*! Clear all published messages */
    void ClearPublishedMessages();

    /*! Get all current subscriptions */
    std::vector<std::string> GetSubscriptions() const;

    /*! Check if subscribed to a topic */
    bool IsSubscribed(const std::string& topic) const;

    /*! Get the QoS for a subscription */
    int GetSubscriptionQos(const std::string& topic) const;

private:
    struct Subscription {
        std::string topic;
        int qos;
        int subscriptionId;
    };

    std::string _clientId;
    mutable std::mutex _mutex;

    // Published messages
    std::vector<stinger::mqtt::Message> _publishedMessages;

    // Subscriptions
    std::map<std::string, Subscription> _subscriptions;
    int _nextSubscriptionId;

    // Callbacks
    std::map<CallbackHandleType, std::function<void(const stinger::mqtt::Message&)>> _callbacks;
    CallbackHandleType _nextCallbackHandle;
};

} // namespace utils
} // namespace stinger
