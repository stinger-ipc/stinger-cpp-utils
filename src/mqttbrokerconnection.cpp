#include "stinger/mqtt/brokerconnection.hpp"
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <mosquitto.h>
#include <mqtt_protocol.h>
#include <sstream>
#include <syslog.h>

using namespace std;

namespace stinger {
namespace mqtt {

BrokerConnection::BrokerConnection(const std::string& host, int port, const std::string& clientId)
    : _mosq(NULL), _host(host), _port(port), _clientId(clientId), _logLevel(LOG_NOTICE) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("Mosquitto lib init problem");
    };
    _mosq = mosquitto_new(_clientId.c_str(), false, (void*)this);
    mosquitto_int_option(_mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);

    mosquitto_log_callback_set(_mosq, [](struct mosquitto* mosq, void* user, int level, const char* str) {
        BrokerConnection* thisClient = static_cast<BrokerConnection*>(user);
        thisClient->Log(level, str);
    });

    mosquitto_connect_v5_callback_set(_mosq, [](struct mosquitto* mosq, void* user, int rc, int flags,
                                                const mosquitto_property* props) {
        BrokerConnection* thisClient = static_cast<BrokerConnection*>(user);

        cout << "Connected to " << thisClient->_host << endl;

        const mosquitto_property* prop;
        for (prop = props; prop != NULL; prop = mosquitto_property_next(prop)) {
            if (mosquitto_property_identifier(prop) == MQTT_PROP_REASON_STRING) {
                char* reasonString = NULL;
                if (mosquitto_property_read_string(prop, MQTT_PROP_REASON_STRING, &reasonString, false)) {
                    thisClient->Log(LOG_INFO, "Connect reason: %s", reasonString);
                    free(reasonString);
                }
            }
        }

        std::lock_guard<std::mutex> lock(thisClient->_mutex);
        while (!thisClient->_subscriptions.empty()) {
            auto sub = thisClient->_subscriptions.front();
            thisClient->Log(LOG_INFO, "Delayed Subscribing to %s as %d", sub.topic.c_str(), sub.subscriptionId);
            mosquitto_property* propList = NULL;
            mosquitto_property_add_varint(&propList, MQTT_PROP_SUBSCRIPTION_IDENTIFIER, sub.subscriptionId);
            int rc = mosquitto_subscribe_v5(mosq, NULL, sub.topic.c_str(), sub.qos, MQTT_SUB_OPT_NO_LOCAL, propList);
            mosquitto_property_free_all(&propList);
            thisClient->_subscriptions.pop();
        }
        while (!thisClient->_msgQueue.empty()) {
            PendingPublish& pending = thisClient->_msgQueue.front();
            Message& msg = pending.message;
            thisClient->Log(LOG_INFO, "Publishing queued message to %s", msg.topic.c_str());
            mosquitto_property* propList = NULL;
            if (msg.properties.contentType) {
                mosquitto_property_add_string(&propList, MQTT_PROP_CONTENT_TYPE, msg.properties.contentType->c_str());
            }
            if (msg.properties.correlationData) {
                mosquitto_property_add_binary(&propList, MQTT_PROP_CORRELATION_DATA,
                                              static_cast<const void*>(msg.properties.correlationData->data()),
                                              msg.properties.correlationData->size());
            }
            if (msg.properties.responseTopic) {
                mosquitto_property_add_string(&propList, MQTT_PROP_RESPONSE_TOPIC,
                                              msg.properties.responseTopic->c_str());
            }
            if (msg.properties.messageExpiryInterval) {
                mosquitto_property_add_int32(&propList, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
                                             *msg.properties.messageExpiryInterval);
            }
            if (msg.properties.debugInfo) {
                mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "DebugInfo",
                                                   msg.properties.debugInfo->c_str());
            }
            if (msg.properties.returnCode) {
                std::string returnCodeStr = std::to_string(static_cast<int>(*msg.properties.returnCode));
                mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "ReturnCode",
                                                   returnCodeStr.c_str());
            }
            if (msg.properties.propertyVersion) {
                std::string propertyVersionStr = std::to_string(*msg.properties.propertyVersion);
                mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "PropertyVersion",
                                                   propertyVersionStr.c_str());
            }
            if (msg.properties.version) {
                mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "Version",
                                                   msg.properties.version->c_str());
            }
            int mid;
            mosquitto_publish_v5(mosq, &mid, msg.topic.c_str(), msg.payload.size(), msg.payload.c_str(), msg.qos,
                                 msg.retain, propList);
            mosquitto_property_free_all(&propList);
            thisClient->_sendMessages[mid] = pending.pSentPromise;
            thisClient->_msgQueue.pop();
        }

        { // Send online message
            auto onlineTopic = thisClient->GetOnlineTopic();
            int mid;
            mosquitto_property* propList = NULL;
            mosquitto_property_add_string(&propList, MQTT_PROP_CONTENT_TYPE, "application/json");
            const char* onlinePayload = "{\"status\":\"online\"}";
            mosquitto_publish_v5(mosq, &mid, onlineTopic.c_str(), sizeof(onlinePayload), onlinePayload, 1, true,
                                 propList);
            mosquitto_property_free_all(&propList);
        }
    });

    mosquitto_disconnect_v5_callback_set(
        _mosq, [](struct mosquitto* mosq, void* user, int rc, const mosquitto_property* props) {
            BrokerConnection* thisClient = static_cast<BrokerConnection*>(user);
            thisClient->Log(LOG_WARNING, "Disconnected from %s with reason code: %d", thisClient->_host.c_str(), rc);

            // Log any disconnect reason from MQTT v5 properties
            const mosquitto_property* prop;
            for (prop = props; prop != NULL; prop = mosquitto_property_next(prop)) {
                if (mosquitto_property_identifier(prop) == MQTT_PROP_REASON_STRING) {
                    char* reasonString = NULL;
                    if (mosquitto_property_read_string(prop, MQTT_PROP_REASON_STRING, &reasonString, false)) {
                        thisClient->Log(LOG_WARNING, "Disconnect reason: %s", reasonString);
                        free(reasonString);
                    }
                }
            }
        });

    mosquitto_message_v5_callback_set(_mosq, [](struct mosquitto* mosq, void* user,
                                                const struct mosquitto_message* mmsg, const mosquitto_property* props) {
        BrokerConnection* thisClient = static_cast<BrokerConnection*>(user);
        thisClient->Log(LOG_DEBUG, "Forwarding message (%s) to %zu callbacks", mmsg->topic,
                        thisClient->_messageCallbacks.size());
        std::string topic(mmsg->topic);
        std::string payload(static_cast<char*>(mmsg->payload), mmsg->payloadlen);
        mqtt::Properties mqttProps;
        const mosquitto_property* prop;
        for (prop = props; prop != NULL; prop = mosquitto_property_next(prop)) {
            if (mosquitto_property_identifier(prop) == MQTT_PROP_CORRELATION_DATA) {
                void* correlation_data;
                uint16_t correlation_data_len;
                if (mosquitto_property_read_binary(prop, MQTT_PROP_CORRELATION_DATA, &correlation_data,
                                                   &correlation_data_len, false)) {
                    std::vector<std::byte> correlationVec;
                    correlationVec.reserve(correlation_data_len);
                    uint8_t* bytePtr = static_cast<uint8_t*>(correlation_data);
                    for (uint16_t i = 0; i < correlation_data_len; ++i) {
                        correlationVec.push_back(static_cast<std::byte>(bytePtr[i]));
                    }
                    mqttProps.correlationData = correlationVec;
                    free(correlation_data);
                }
            } else if (mosquitto_property_identifier(prop) == MQTT_PROP_RESPONSE_TOPIC) {
                char* responseTopic = NULL;
                if (mosquitto_property_read_string(prop, MQTT_PROP_RESPONSE_TOPIC, &responseTopic, false)) {
                    mqttProps.responseTopic = std::string(responseTopic);
                    free(responseTopic);
                }
            } else if (mosquitto_property_identifier(prop) == MQTT_PROP_USER_PROPERTY) {
                char* name = NULL;
                char* value = NULL;
                if (mosquitto_property_read_string_pair(prop, MQTT_PROP_USER_PROPERTY, &name, &value, false)) {
                    if (strcmp(name, "ReturnValue") == 0) {
                        int returnValueInt = std::stoi(value);
                        mqttProps.returnCode = returnValueInt;
                    } else if (strcmp(name, "PropertyVersion") == 0) {
                        int propertyVersionInt = std::stoi(value);
                        mqttProps.propertyVersion = propertyVersionInt;
                    } else if (strcmp(name, "DebugInfo") == 0) {
                        mqttProps.debugInfo = std::string(value);
                    } else if (strcmp(name, "Version") == 0) {
                        mqttProps.version = std::string(value);
                    }
                    free(name);
                    free(value);
                }
            } else if (mosquitto_property_identifier(prop) == MQTT_PROP_SUBSCRIPTION_IDENTIFIER) {
                uint32_t subscriptionId;
                if (mosquitto_property_read_varint(prop, MQTT_PROP_SUBSCRIPTION_IDENTIFIER, &subscriptionId, false)) {
                    mqttProps.subscriptionId = subscriptionId;
                }
            } else if (mosquitto_property_identifier(prop) == MQTT_PROP_CONTENT_TYPE) {
                char* contentType = NULL;
                if (mosquitto_property_read_string(prop, MQTT_PROP_CONTENT_TYPE, &contentType, false)) {
                    mqttProps.contentType = std::string(contentType);
                    free(contentType);
                }
            } else if (mosquitto_property_identifier(prop) == MQTT_PROP_MESSAGE_EXPIRY_INTERVAL) {
                uint32_t messageExpiryInterval;
                if (mosquitto_property_read_int32(prop, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL, &messageExpiryInterval,
                                                  false)) {
                    mqttProps.messageExpiryInterval = messageExpiryInterval;
                }
            }
        }
        auto msg = Message(topic, payload, mmsg->qos, mmsg->retain, mqttProps);
        for (const auto& entry : thisClient->_messageCallbacks) {
            thisClient->Log(LOG_DEBUG, "Calling callback (handle=%d) for topic: %s", static_cast<int>(entry.first),
                            topic.c_str());
            const auto& cb = entry.second;
            cb(msg);
        }
    });

    mosquitto_publish_v5_callback_set(
        _mosq, [](struct mosquitto* mosq, void* user, int mid, int reason_code, const mosquitto_property* props) {
            BrokerConnection* thisClient = static_cast<BrokerConnection*>(user);
            auto found = thisClient->_sendMessages.find(mid);
            if (found != thisClient->_sendMessages.end()) {
                found->second->set_value(true);
                thisClient->_sendMessages.erase(found);
            }
            thisClient->Log(LOG_DEBUG, "Publish completed for mid=%d, reason_code=%d", mid, reason_code);
        });

    Connect();
    mosquitto_loop_start(_mosq);
}

BrokerConnection::~BrokerConnection() {
    std::lock_guard<std::mutex> lock(_mutex);
    mosquitto_loop_stop(_mosq, true);
    mosquitto_disconnect(_mosq);
    mosquitto_destroy(_mosq);
    mosquitto_lib_cleanup();
}

void BrokerConnection::Connect() {
    auto onlineTopic = GetOnlineTopic();
    mosquitto_property* propList = NULL;
    mosquitto_property_add_string(&propList, MQTT_PROP_CONTENT_TYPE, "application/json");
    const char* offlinePayload = "{\"status\":\"offline\"}";
    int will_rc = mosquitto_will_set_v5(_mosq, onlineTopic.c_str(), sizeof(offlinePayload), offlinePayload,
                                        1,    // qos
                                        true, // retain
                                        propList);

    // If will_set failed, free the properties we created. On success the
    // mosquitto library takes ownership of the property list, so do not free it.
    if (will_rc != MOSQ_ERR_SUCCESS) {
        Log(LOG_ERR, "Failed to set will message: %d", will_rc);
        if (propList) {
            mosquitto_property_free_all(&propList);
            propList = NULL;
        }
    }

    int rc = mosquitto_connect_bind_v5(_mosq, _host.c_str(), _port, 120, NULL, NULL);
    if (rc != MOSQ_ERR_SUCCESS) {
        Log(LOG_ERR, "Failed to connect to MQTT broker: %d", rc);
    }
}

std::future<bool> BrokerConnection::Publish(const Message& message) {
    int mid;
    mosquitto_property* propList = NULL;
    if (message.properties.contentType) {
        mosquitto_property_add_string(&propList, MQTT_PROP_CONTENT_TYPE, message.properties.contentType->c_str());
    }
    if (message.properties.correlationData) {
        mosquitto_property_add_binary(&propList, MQTT_PROP_CORRELATION_DATA,
                                      static_cast<const void*>(message.properties.correlationData->data()),
                                      message.properties.correlationData->size());
    }
    if (message.properties.responseTopic) {
        mosquitto_property_add_string(&propList, MQTT_PROP_RESPONSE_TOPIC, message.properties.responseTopic->c_str());
    }
    if (message.properties.messageExpiryInterval) {
        mosquitto_property_add_int32(&propList, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
                                     *message.properties.messageExpiryInterval);
    }
    if (message.properties.debugInfo) {
        mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "DebugInfo",
                                           message.properties.debugInfo->c_str());
    }
    if (message.properties.returnCode) {
        std::string returnCodeStr = std::to_string(static_cast<int>(*message.properties.returnCode));
        mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "ReturnCode", returnCodeStr.c_str());
    }
    if (message.properties.propertyVersion) {
        std::string propertyVersionStr = std::to_string(static_cast<int>(*message.properties.propertyVersion));
        mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "PropertyVersion",
                                           propertyVersionStr.c_str());
    }
    if (message.properties.version) {
        mosquitto_property_add_string_pair(&propList, MQTT_PROP_USER_PROPERTY, "Version",
                                           message.properties.version->c_str());
    }
    int rc = mosquitto_publish_v5(_mosq, &mid, message.topic.c_str(), message.payload.size(), message.payload.c_str(),
                                  message.qos, message.retain, propList);
    if (propList) {
        mosquitto_property_free_all(&propList);
    }
    if (rc == MOSQ_ERR_NO_CONN) {
        Log(LOG_DEBUG, "Delayed published queued to: %s", message.topic.c_str());
        std::lock_guard<std::mutex> lock(_mutex);
        auto pending = PendingPublish(message);
        auto future = pending.GetFuture();
        _msgQueue.push(pending);
        return future;
    } else if (rc == MOSQ_ERR_SUCCESS) {
        Log(LOG_INFO, "Published to: %s | %s", message.topic.c_str(), message.payload.c_str());
        auto pPromise = std::make_shared<std::promise<bool>>();
        auto future = pPromise->get_future();
        std::lock_guard<std::mutex> lock(_mutex);
        _sendMessages[mid] = std::move(pPromise);
        return future;
    } else {
        Log(LOG_ERR, "Failed to publish to %s: rc=%d", message.topic.c_str(), rc);
    }
    throw std::runtime_error("Unhandled rc");
}

int BrokerConnection::Subscribe(const std::string& topic, int qos) {
    std::lock_guard<std::mutex> lock(_mutex);

    // Check if we already have a subscription for this topic
    auto it = _subscriptionRefCounts.find(topic);
    if (it != _subscriptionRefCounts.end()) {
        // Topic already subscribed - increment reference count
        it->second.first++;
        Log(LOG_DEBUG, "Incremented subscription count for %s to %d", topic.c_str(), it->second.first);
        return it->second.second; // Return existing subscription ID
    }

    // New subscription - create it
    int subscriptionId = _nextSubscriptionId++;
    mosquitto_property* propList = NULL;
    mosquitto_property_add_varint(&propList, MQTT_PROP_SUBSCRIPTION_IDENTIFIER, subscriptionId);
    int rc = mosquitto_subscribe_v5(_mosq, NULL, topic.c_str(), qos, MQTT_SUB_OPT_NO_LOCAL, propList);
    mosquitto_property_free_all(&propList);

    if (rc == MOSQ_ERR_NO_CONN) {
        Log(LOG_DEBUG, "Subscription %d queued for: %s", subscriptionId, topic.c_str());
        BrokerConnection::MqttSubscription sub(topic, qos, subscriptionId);
        _subscriptions.push(sub);
        // Store ref count as 1 for queued subscription
        _subscriptionRefCounts[topic] = std::make_pair(1, subscriptionId);
    } else if (rc == MOSQ_ERR_SUCCESS) {
        Log(LOG_INFO, "Online Subscribed to %s as %d", topic.c_str(), subscriptionId);
        // Store ref count as 1 for active subscription
        _subscriptionRefCounts[topic] = std::make_pair(1, subscriptionId);
    }

    return subscriptionId;
}

void BrokerConnection::Unsubscribe(const std::string& topic) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _subscriptionRefCounts.find(topic);
    if (it == _subscriptionRefCounts.end()) {
        Log(LOG_WARNING, "Attempted to unsubscribe from topic %s that was never subscribed", topic.c_str());
        return;
    }

    // Decrement reference count
    it->second.first--;

    if (it->second.first > 0) {
        // Still have active references - just decrement
        Log(LOG_DEBUG, "Decremented subscription count for %s to %d", topic.c_str(), it->second.first);
        return;
    }

    // Reference count reached 0 - perform actual unsubscription
    Log(LOG_DEBUG, "Unsubscribing from %s (ref count reached 0)", topic.c_str());
    int rc = mosquitto_unsubscribe(_mosq, NULL, topic.c_str());

    if (rc != MOSQ_ERR_SUCCESS) {
        Log(LOG_WARNING, "Failed to unsubscribe from %s: rc=%d", topic.c_str(), rc);
    }

    // Remove from tracking map
    _subscriptionRefCounts.erase(it);
}

utils::CallbackHandleType BrokerConnection::AddMessageCallback(const std::function<void(const Message&)>& cb) {
    std::lock_guard<std::mutex> lock(_mutex);
    utils::CallbackHandleType handle = _nextCallbackHandle++;
    _messageCallbacks[handle] = cb;
    Log(LOG_DEBUG, "Message callback set with handle %d", handle);
    return handle;
}

void BrokerConnection::RemoveMessageCallback(utils::CallbackHandleType handle) {
    if (handle > 0) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto found = _messageCallbacks.find(handle);
        if (found != _messageCallbacks.end()) {
            _messageCallbacks.erase(found);
            Log(LOG_DEBUG, "Removed message callback with handle %d", handle);
        } else {
            Log(LOG_WARNING, "No message callback found with handle %d", handle);
        }
    }
}

bool BrokerConnection::TopicMatchesSubscription(const std::string& topic, const std::string& subscr) const {
    bool result;
    int rc = mosquitto_topic_matches_sub(subscr.c_str(), topic.c_str(), &result);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("Mosquitto error");
    }
    return result;
}

std::string BrokerConnection::GetClientId() const {
    return _clientId;
}

std::string BrokerConnection::GetOnlineTopic() const {
    return "client/" + _clientId + "/online";
}

void BrokerConnection::SetLogFunction(const utils::LogFunctionType& logFunc) {
    _logger = logFunc;
}

void BrokerConnection::SetLogLevel(int level) {
    _logLevel = level;
}

void BrokerConnection::Log(int level, const char* fmt, ...) const {
    if (_logger && (level <= _logLevel)) {
        va_list args;
        va_start(args, fmt);
        char buf[256];
        vsnprintf(buf, sizeof(buf), fmt, args);
        std::string msg(buf);
        va_end(args);
        _logger(level, msg.c_str());
    }
}

} // namespace mqtt
} // namespace stinger
