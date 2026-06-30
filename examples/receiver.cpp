#include "stinger/mqtt/brokerconnection.hpp"
#include "stinger/mqtt/message.hpp"
#include "stinger/utils/conversions.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <syslog.h>
#include <thread>

using namespace stinger;

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <topic>" << std::endl;
        std::cerr << "Example: " << argv[0] << " hello/world" << std::endl;
        return 1;
    }

    std::string topic = argv[1];

    // Create MQTT connection to localhost broker
    // Parameters: host, port, clientId
    auto mqtt = std::make_unique<mqtt::BrokerConnection>("localhost", 1883, "receiver_client");

    // Set up logging (optional)
    mqtt->SetLogFunction([](int level, const char* message) { std::cout << "[MQTT Log] " << message << std::endl; });
    mqtt->SetLogLevel(LOG_DEBUG);

    // Add a message callback to handle received messages
    auto callbackHandle = mqtt->AddMessageCallback([](const mqtt::Message& msg) {
        std::cout << "\n========== Message Received ==========" << std::endl;
        std::cout << "Topic: " << msg.topic << std::endl;
        std::cout << "Payload: " << utils::toString(msg.payload) << std::endl;
        std::cout << "QoS: " << msg.qos << std::endl;
        std::cout << "Retain: " << (msg.retain ? "true" : "false") << std::endl;

        std::cout << "\n--- Properties ---" << std::endl;

        // Check correlation data
        if (msg.properties.correlationData) {
            std::cout << "Correlation Data: " << msg.properties.correlationData->size() << " bytes" << std::endl;
        } else {
            std::cout << "Correlation Data: (none)" << std::endl;
        }

        // Check response topic
        if (msg.properties.responseTopic) {
            std::cout << "Response Topic: " << *msg.properties.responseTopic << std::endl;
        } else {
            std::cout << "Response Topic: (none)" << std::endl;
        }

        // Check subscription ID
        if (msg.properties.subscriptionId) {
            std::cout << "Subscription ID: " << *msg.properties.subscriptionId << std::endl;
        } else {
            std::cout << "Subscription ID: (none)" << std::endl;
        }

        // Check message expiry interval
        if (msg.properties.messageExpiryInterval) {
            std::cout << "Message Expiry Interval: " << *msg.properties.messageExpiryInterval << " seconds"
                      << std::endl;
        } else {
            std::cout << "Message Expiry Interval: (none)" << std::endl;
        }

        // Check content type
        if (msg.properties.contentType) {
            std::cout << "Content Type: " << *msg.properties.contentType << std::endl;
        } else {
            std::cout << "Content Type: (none)" << std::endl;
        }

        // Check debug info
        if (msg.properties.debugInfo) {
            std::cout << "Debug Info: " << *msg.properties.debugInfo << std::endl;
        } else {
            std::cout << "Debug Info: (none)" << std::endl;
        }

        // Check return code
        if (msg.properties.returnCode) {
            std::cout << "Return Code: " << *msg.properties.returnCode << std::endl;
        } else {
            std::cout << "Return Code: (none)" << std::endl;
        }

        // Check property version
        if (msg.properties.propertyVersion) {
            std::cout << "Property Version: " << *msg.properties.propertyVersion << std::endl;
        } else {
            std::cout << "Property Version: (none)" << std::endl;
        }

        // Check version
        if (msg.properties.version) {
            std::cout << "Version: " << *msg.properties.version << std::endl;
        } else {
            std::cout << "Version: (none)" << std::endl;
        }

        std::cout << "======================================\n" << std::endl;
    });

    // Subscribe to the specified topic with QoS 1
    int subscriptionId = mqtt->Subscribe(topic, 1);
    std::cout << "Subscribed to '" << topic << "' with subscription ID: " << subscriptionId << std::endl;
    std::cout << "Waiting for messages... (Press Ctrl+C to exit)" << std::endl;

    // Wait indefinitely for messages
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up: remove callback and unsubscribe
    mqtt->RemoveMessageCallback(callbackHandle);
    mqtt->Unsubscribe(topic);

    std::cout << "Exiting..." << std::endl;
    return 0;
}
