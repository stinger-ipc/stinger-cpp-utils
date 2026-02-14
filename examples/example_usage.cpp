#include "stinger/utils/mqttbrokerconnection.hpp"
#include "stinger/utils/mqttmessage.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <syslog.h>
#include <thread>

using namespace stinger::utils;

int main() {
    // Create MQTT connection to broker
    // Parameters: host, port, clientId
    auto mqtt = std::make_unique<MqttBrokerConnection>("localhost", 1883, "example_client");

    // Set up logging (optional)
    mqtt->SetLogFunction([](int level, const char* message) { std::cout << "[MQTT Log] " << message << std::endl; });
    mqtt->SetLogLevel(LOG_DEBUG);

    // Add a message callback to handle received messages
    auto callbackHandle = mqtt->AddMessageCallback([](const MqttMessage& msg) {
        std::cout << "Received message on topic: " << msg.topic << std::endl;
        std::cout << "Payload: " << msg.payload << std::endl;

        // Check if there's correlation data
        if (msg.properties.correlationData) {
            std::cout << "Correlation data present (" << msg.properties.correlationData->size() << " bytes)" << std::endl;
        }

        // Check if there's a response topic
        if (msg.properties.responseTopic) {
            std::cout << "Response Topic: " << *msg.properties.responseTopic << std::endl;
        }
    });

    // Subscribe to the topic "hello/world" with QoS 1
    int subscriptionId = mqtt->Subscribe("hello/world", 1);
    std::cout << "Subscribed to hello/world with subscription ID: " << subscriptionId << std::endl;

    // Give the connection a moment to establish
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Create and publish a message to "hello/publish"
    auto msg = MqttMessage::Signal("hello/publish", "Hello from example_usage!");

    // Optionally set properties
    msg.properties.contentType = "text/plain";

    // Publish the message and get a future
    auto publishFuture = mqtt->Publish(msg);

    std::cout << "Publishing message to hello/publish..." << std::endl;

    // Wait for publish confirmation
    if (publishFuture.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
        if (publishFuture.get()) {
            std::cout << "Message published successfully!" << std::endl;
        }
    } else {
        std::cout << "Publish timeout" << std::endl;
    }

    // Wait 30 seconds to receive messages
    std::cout << "Waiting 30 seconds for messages..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Clean up: remove callback and unsubscribe
    mqtt->RemoveMessageCallback(callbackHandle);
    mqtt->Unsubscribe("hello/world");

    std::cout << "Exiting..." << std::endl;
    return 0;
}
