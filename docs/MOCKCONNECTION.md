# MockConnection - Testing Guide

## Overview

`MockConnection` is a test implementation of `IConnection` that doesn't require an actual MQTT broker. It's designed for unit testing libraries that depend on MQTT connectivity.

## Building with MockConnection

By default, `MockConnection` is **not** compiled into the library. To enable it:

```bash
mkdir build && cd build
cmake -DSTINGER_UTILS_BUILD_MOCK=ON ..
cmake --build .
```

## Basic Usage

```cpp
#include <stinger/utils/mockconnection.hpp>
#include <stinger/utils/mqttmessage.hpp>

using namespace stinger::utils;

// Create a mock connection
auto mock = std::make_unique<MockConnection>("test_client");

// Subscribe to topics
mock->Subscribe("sensor/temperature", 1);

// Add callbacks
auto handle = mock->AddMessageCallback([](const mqtt::Message& msg) {
    std::cout << "Received: " << msg.topic << " = " << msg.payload << std::endl;
});

// Publish messages (they get queued internally)
auto msg = mqtt::Message::Signal("sensor/temperature", "22.5");
mock->Publish(msg);

// Simulate receiving messages (triggers callbacks)
auto incoming = mqtt::Message::Signal("sensor/temperature", "23.0");
mock->SimulateIncomingMessage(incoming);
```

## Testing Utilities

### Verify Published Messages

```cpp
// Get all published messages
auto allMessages = mock->GetPublishedMessages();
EXPECT_EQ(allMessages.size(), 1);

// Get messages for a specific topic
auto tempMessages = mock->GetPublishedMessages("sensor/temperature");
EXPECT_EQ(tempMessages[0].payload, "22.5");

// Clear published messages
mock->ClearPublishedMessages();
```

### Verify Subscriptions

```cpp
// Check if subscribed
EXPECT_TRUE(mock->IsSubscribed("sensor/temperature"));

// Get QoS level
EXPECT_EQ(mock->GetSubscriptionQos("sensor/temperature"), 1);

// Get all subscriptions
auto subs = mock->GetSubscriptions();
EXPECT_EQ(subs.size(), 1);
```

### Simulate Incoming Messages

```cpp
// Add a callback to capture messages
bool messageReceived = false;
mqtt::Message receivedMsg;

mock->AddMessageCallback([&](const mqtt::Message& msg) {
    messageReceived = true;
    receivedMsg = msg;
});

mock->Subscribe("test/topic", 1);

// Simulate an incoming message
auto msg = mqtt::Message::Signal("test/topic", "hello");
mock->SimulateIncomingMessage(msg);

EXPECT_TRUE(messageReceived);
EXPECT_EQ(receivedMsg.payload, "hello");
```

## Google Test Example

```cpp
#include <gtest/gtest.h>
#include <stinger/utils/mockconnection.hpp>

class MyServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockConnection = std::make_unique<MockConnection>("test");
        // Initialize your service with the mock connection
    }

    std::unique_ptr<MockConnection> mockConnection;
};

TEST_F(MyServiceTest, PublishesCorrectMessage) {
    // Arrange: set up test conditions
    
    // Act: perform the action
    // myService->doSomething();
    
    // Assert: verify results
    auto messages = mockConnection->GetPublishedMessages("expected/topic");
    ASSERT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0].payload, "expected payload");
}

TEST_F(MyServiceTest, HandlesIncomingMessages) {
    // Subscribe and trigger callback in your service
    
    // Simulate an incoming message
    auto msg = mqtt::Message::Signal("command/topic", "start");
    mockConnection->SimulateIncomingMessage(msg);
    
    // Verify your service responded correctly
    // ...
}
```

## Topic Matching

`MockConnection` supports basic MQTT wildcards:

```cpp
// Single-level wildcard (+)
EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp", "sensor/+"));

// Multi-level wildcard (#)
EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp/room1", "sensor/#"));
EXPECT_TRUE(mock->TopicMatchesSubscription("any/topic", "#"));
```

## Using in CMake Projects

If your project depends on StingerUtils and needs mock support:

```cmake
find_package(StingerUtils REQUIRED)

# Your test executable
add_executable(my_tests test_main.cpp)

# Link with StingerUtils (built with mock support)
target_link_libraries(my_tests PRIVATE Stinger::Utils GTest::gtest_main)
```

Make sure to build StingerUtils with `-DSTINGER_UTILS_BUILD_MOCK=ON` when installing it for test environments.

## Thread Safety

`MockConnection` is thread-safe. All methods use internal mutex locking to ensure safe concurrent access.

## Limitations

- Topic matching implements basic wildcards (`+` and `#`) but may not support all MQTT 5.0 features
- Messages are stored in memory - be mindful of memory usage in long-running tests
- No actual network communication occurs
