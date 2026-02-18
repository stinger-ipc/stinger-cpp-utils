#include "stinger/mqtt/message.hpp"
#include "stinger/utils/mockconnection.hpp"
#include <gtest/gtest.h>

using namespace stinger;

class MockConnectionTest : public ::testing::Test {
protected:
    void SetUp() override { mock = std::make_unique<MockConnection>("test_client"); }

    std::unique_ptr<MockConnection> mock;
};

TEST_F(MockConnectionTest, PublishMessage) {
    auto msg = mqtt::Message::Signal("test/topic", "test payload");
    auto future = mock->Publish(msg);

    // Should complete immediately
    ASSERT_TRUE(future.get());

    // Check message was recorded
    auto published = mock->GetPublishedMessages();
    ASSERT_EQ(published.size(), 1);
    EXPECT_EQ(published[0].topic, "test/topic");
    EXPECT_EQ(published[0].payload, "test payload");
}

TEST_F(MockConnectionTest, Subscribe) {
    int subId = mock->Subscribe("sensor/temperature", 1);

    EXPECT_GT(subId, 0);
    EXPECT_TRUE(mock->IsSubscribed("sensor/temperature"));
    EXPECT_EQ(mock->GetSubscriptionQos("sensor/temperature"), 1);
}

TEST_F(MockConnectionTest, Unsubscribe) {
    mock->Subscribe("sensor/temperature", 1);
    EXPECT_TRUE(mock->IsSubscribed("sensor/temperature"));

    mock->Unsubscribe("sensor/temperature");
    EXPECT_FALSE(mock->IsSubscribed("sensor/temperature"));
}

TEST_F(MockConnectionTest, MessageCallback) {
    bool callbackCalled = false;
    mqtt::Message receivedMsg;

    auto handle = mock->AddMessageCallback([&](const mqtt::Message& msg) {
        callbackCalled = true;
        receivedMsg = msg;
    });

    mock->Subscribe("test/topic", 1);

    auto msg = mqtt::Message::Signal("test/topic", "hello");
    mock->SimulateIncomingMessage(msg);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedMsg.topic, "test/topic");
    EXPECT_EQ(receivedMsg.payload, "hello");
}

TEST_F(MockConnectionTest, WildcardSubscription) {
    bool callbackCalled = false;

    auto handle = mock->AddMessageCallback([&](const mqtt::Message& msg) { callbackCalled = true; });

    mock->Subscribe("sensor/#", 1);

    auto msg = mqtt::Message::Signal("sensor/temperature", "22.5");
    mock->SimulateIncomingMessage(msg);

    EXPECT_TRUE(callbackCalled);
}

TEST_F(MockConnectionTest, GetPublishedMessagesByTopic) {
    mock->Publish(mqtt::Message::Signal("topic1", "msg1"));
    mock->Publish(mqtt::Message::Signal("topic2", "msg2"));
    mock->Publish(mqtt::Message::Signal("topic1", "msg3"));

    auto topic1Messages = mock->GetPublishedMessages("topic1");
    ASSERT_EQ(topic1Messages.size(), 2);
    EXPECT_EQ(topic1Messages[0].payload, "msg1");
    EXPECT_EQ(topic1Messages[1].payload, "msg3");
}

TEST_F(MockConnectionTest, ClearPublishedMessages) {
    mock->Publish(mqtt::Message::Signal("test/topic", "test"));
    ASSERT_EQ(mock->GetPublishedMessages().size(), 1);

    mock->ClearPublishedMessages();
    EXPECT_EQ(mock->GetPublishedMessages().size(), 0);
}

TEST_F(MockConnectionTest, GetClientId) {
    EXPECT_EQ(mock->GetClientId(), "test_client");
}

TEST_F(MockConnectionTest, GetOnlineTopic) {
    EXPECT_EQ(mock->GetOnlineTopic(), "test_client/online");
}

TEST_F(MockConnectionTest, TopicMatching) {
    EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp", "sensor/temp"));
    EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp", "sensor/+"));
    EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp", "#"));
    EXPECT_TRUE(mock->TopicMatchesSubscription("sensor/temp/room1", "sensor/#"));
    EXPECT_FALSE(mock->TopicMatchesSubscription("sensor/temp", "device/temp"));
}
