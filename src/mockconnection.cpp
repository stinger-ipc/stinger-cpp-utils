#include "stinger/utils/mockconnection.hpp"
#include <algorithm>
#include <cstdarg>
#include <iostream>

namespace stinger {
namespace utils {

MockConnection::MockConnection(const std::string& clientId)
    : _clientId(clientId), _nextSubscriptionId(1), _nextCallbackHandle(1) {}

MockConnection::~MockConnection() {}

std::future<bool> MockConnection::Publish(const MqttMessage& mqttMsg) {
    std::lock_guard<std::mutex> lock(_mutex);
    _publishedMessages.push_back(mqttMsg);

    // Create a promise that immediately resolves to true
    std::promise<bool> promise;
    promise.set_value(true);
    return promise.get_future();
}

int MockConnection::Subscribe(const std::string& topic, int qos) {
    std::lock_guard<std::mutex> lock(_mutex);

    int subscriptionId = _nextSubscriptionId++;
    Subscription sub;
    sub.topic = topic;
    sub.qos = qos;
    sub.subscriptionId = subscriptionId;

    _subscriptions[topic] = sub;
    return subscriptionId;
}

void MockConnection::Unsubscribe(const std::string& topic) {
    std::lock_guard<std::mutex> lock(_mutex);
    _subscriptions.erase(topic);
}

CallbackHandleType MockConnection::AddMessageCallback(const std::function<void(const MqttMessage&)>& cb) {
    std::lock_guard<std::mutex> lock(_mutex);
    CallbackHandleType handle = _nextCallbackHandle++;
    _callbacks[handle] = cb;
    return handle;
}

void MockConnection::RemoveMessageCallback(CallbackHandleType handle) {
    std::lock_guard<std::mutex> lock(_mutex);
    _callbacks.erase(handle);
}

bool MockConnection::TopicMatchesSubscription(const std::string& topic, const std::string& subscr) const {
    // Simple implementation - for more complex matching, use mosquitto_topic_matches_sub
    // This handles basic wildcards: + (single level) and # (multi level)

    if (subscr == "#") {
        return true; // Match everything
    }

    if (subscr == topic) {
        return true; // Exact match
    }

    // Split both topic and subscription by '/'
    auto split = [](const std::string& s) -> std::vector<std::string> {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = s.find('/');
        while (end != std::string::npos) {
            result.push_back(s.substr(start, end - start));
            start = end + 1;
            end = s.find('/', start);
        }
        result.push_back(s.substr(start));
        return result;
    };

    auto topicParts = split(topic);
    auto subscrParts = split(subscr);

    size_t ti = 0, si = 0;
    while (ti < topicParts.size() && si < subscrParts.size()) {
        if (subscrParts[si] == "#") {
            return true; // # matches everything remaining
        }

        if (subscrParts[si] != "+" && subscrParts[si] != topicParts[ti]) {
            return false; // Mismatch
        }

        ti++;
        si++;
    }

    // Both must be fully consumed
    return ti == topicParts.size() && si == subscrParts.size();
}

std::string MockConnection::GetClientId() const {
    return _clientId;
}

std::string MockConnection::GetOnlineTopic() const {
    return _clientId + "/online";
}

void MockConnection::Log(int level, const char* fmt, ...) const {
    // Simple logging implementation
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::cout << "[MockConnection] " << buffer << std::endl;
}

// Testing utility methods

void MockConnection::SimulateIncomingMessage(const MqttMessage& msg) {
    std::lock_guard<std::mutex> lock(_mutex);

    // Find matching subscriptions and trigger callbacks
    for (const auto& [topic, sub] : _subscriptions) {
        if (TopicMatchesSubscription(msg.topic, sub.topic)) {
            // Trigger all callbacks
            for (const auto& [handle, callback] : _callbacks) {
                callback(msg);
            }
            break;
        }
    }
}

std::vector<MqttMessage> MockConnection::GetPublishedMessages() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _publishedMessages;
}

std::vector<MqttMessage> MockConnection::GetPublishedMessages(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<MqttMessage> result;

    for (const auto& msg : _publishedMessages) {
        if (msg.topic == topic) {
            result.push_back(msg);
        }
    }

    return result;
}

void MockConnection::ClearPublishedMessages() {
    std::lock_guard<std::mutex> lock(_mutex);
    _publishedMessages.clear();
}

std::vector<std::string> MockConnection::GetSubscriptions() const {
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<std::string> result;

    for (const auto& [topic, sub] : _subscriptions) {
        result.push_back(topic);
    }

    return result;
}

bool MockConnection::IsSubscribed(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _subscriptions.find(topic) != _subscriptions.end();
}

int MockConnection::GetSubscriptionQos(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _subscriptions.find(topic);
    if (it != _subscriptions.end()) {
        return it->second.qos;
    }
    return -1; // Not subscribed
}

} // namespace utils
} // namespace stinger
