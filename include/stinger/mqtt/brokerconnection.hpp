#pragma once

#include "stinger/mqtt/message.hpp"
#include "stinger/utils/iconnection.hpp"
#include <mosquitto.h>

#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace stinger {
namespace mqtt {

/**
 * @brief An Mqtt Connection, implementing IConnection.
 */
class BrokerConnection : public utils::IConnection {
public:
    /*! Constructor for a BrokerConnection.
     * \param hostname IP address or hostname of the MQTT broker server.
     * \param port Port where the MQTT broker is running (often 1883).
     */
    BrokerConnection(const std::string& host, int port, const std::string& clientId);

    virtual ~BrokerConnection();

    /*! Publish a message to the MQTT broker.
     * \param message The MQTT message to publish.
     * \return A future which is resolved to true when the message has been published to the MQTT broker.
     */
    virtual std::future<bool> Publish(const Message& message);

    /*! Subscribe to a topic.
     * \param topic the subscription topic.
     * \param qos an MQTT quality of service value between 0 and 2 inclusive.
     * \return the MQTT subscription ID.
     */
    virtual int Subscribe(const std::string& topic, int qos);

    virtual void Unsubscribe(const std::string& topic);

    /*! Add a function that is called on the receipt of a message.
     * Many callbacks can be added, and each will be called in the order in which the callbacks were added.
     * \param cb the callback function.
     */
    virtual utils::CallbackHandleType AddMessageCallback(const std::function<void(const Message&)>& cb);

    virtual void RemoveMessageCallback(utils::CallbackHandleType handle);

    /*! Determines if a topic string matches a subscription topic.
     * \param topic a topic to match against a subscription.
     * \param subscr the subscription topic string to match against.
     * \return true if it is a match.
     */
    virtual bool TopicMatchesSubscription(const std::string& topic, const std::string& subscr) const;

    virtual std::string GetClientId() const;

    virtual std::string GetOnlineTopic() const;

    virtual void SetLogFunction(const utils::LogFunctionType& logFunc);
    virtual void SetLogLevel(int level);
    virtual void Log(int level, const char* fmt, ...) const;

protected:
    /*! Establishes the connection to the broker.
     */
    virtual void Connect();

private:
    // Represents an MQTT subscription, so that it can be queued before connection.
    struct MqttSubscription {
        MqttSubscription(const std::string& topic, int qos, int subscriptionId)
            : topic(topic), qos(qos), subscriptionId(subscriptionId) {}
        ~MqttSubscription() = default;
        std::string topic;
        int qos;
        int subscriptionId;
    };

    struct PendingPublish {
        PendingPublish(Message msg) : message(std::move(msg)), pSentPromise(std::make_shared<std::promise<bool>>()) {}
        ~PendingPublish() = default;
        Message message;
        std::shared_ptr<std::promise<bool>> pSentPromise;
        std::future<bool> GetFuture() { return pSentPromise->get_future(); }
    };

    mosquitto* _mosq;
    std::string _host;
    int _port;
    std::string _clientId;
    int _nextSubscriptionId = 1;
    std::queue<MqttSubscription> _subscriptions;
    std::mutex _mutex;
    utils::CallbackHandleType _nextCallbackHandle = 1;
    std::map<utils::CallbackHandleType, std::function<void(const Message&)>> _messageCallbacks;
    std::queue<PendingPublish> _msgQueue;
    std::map<int, std::shared_ptr<std::promise<bool>>> _sendMessages;

    // Track subscription reference counts: topic -> (count, subscriptionId)
    std::map<std::string, std::pair<int, int>> _subscriptionRefCounts;

    utils::LogFunctionType _logger;
    int _logLevel = 0;
};

} // namespace mqtt
} // namespace stinger
