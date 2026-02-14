#include "stinger/utils/mqttmessage.hpp"
#include "stinger/utils/mqttproperties.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

using namespace stinger::utils;

// Test MqttMessage construction
TEST(MqttMessageTest, DefaultConstructor) {
    MqttMessage msg("test/topic", "test payload");

    EXPECT_EQ(msg.topic, "test/topic");
    EXPECT_EQ(msg.payload, "test payload");
    EXPECT_EQ(msg.qos, 0);
    EXPECT_FALSE(msg.retain);
}

TEST(MqttMessageTest, ConstructorWithAllParameters) {
    MqttProperties props;
    props.contentType = "application/json";

    MqttMessage msg("test/topic", "test payload", 2, true, props);

    EXPECT_EQ(msg.topic, "test/topic");
    EXPECT_EQ(msg.payload, "test payload");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_TRUE(msg.retain);
    ASSERT_TRUE(msg.properties.contentType.has_value());
    EXPECT_EQ(*msg.properties.contentType, "application/json");
}

TEST(MqttMessageTest, CopyConstructor) {
    MqttMessage original("test/topic", "payload", 1, true);
    original.properties.contentType = "text/plain";

    MqttMessage copy(original);

    EXPECT_EQ(copy.topic, original.topic);
    EXPECT_EQ(copy.payload, original.payload);
    EXPECT_EQ(copy.qos, original.qos);
    EXPECT_EQ(copy.retain, original.retain);
    ASSERT_TRUE(copy.properties.contentType.has_value());
    EXPECT_EQ(*copy.properties.contentType, "text/plain");
}

// Test Signal factory method
TEST(MqttMessageTest, SignalFactoryMethod) {
    auto msg = MqttMessage::Signal("sensors/temp", "22.5");

    EXPECT_EQ(msg.topic, "sensors/temp");
    EXPECT_EQ(msg.payload, "22.5");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_FALSE(msg.retain);
}

// Test PropertyValue factory method
TEST(MqttMessageTest, PropertyValueFactoryMethod) {
    auto msg = MqttMessage::PropertyValue("device/status", "online", 5);

    EXPECT_EQ(msg.topic, "device/status");
    EXPECT_EQ(msg.payload, "online");
    EXPECT_EQ(msg.qos, 1);
    EXPECT_TRUE(msg.retain);
    ASSERT_TRUE(msg.properties.propertyVersion.has_value());
    EXPECT_EQ(*msg.properties.propertyVersion, 5);
}

// Test PropertyUpdateRequest factory method
TEST(MqttMessageTest, PropertyUpdateRequestFactoryMethod) {
    std::vector<std::byte> correlationId = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    std::string responseTopic = "response/topic";

    auto msg = MqttMessage::PropertyUpdateRequest("prop/update", "new_value", 10, correlationId, responseTopic);

    EXPECT_EQ(msg.topic, "prop/update");
    EXPECT_EQ(msg.payload, "new_value");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_FALSE(msg.retain);

    ASSERT_TRUE(msg.properties.propertyVersion.has_value());
    EXPECT_EQ(*msg.properties.propertyVersion, 10);

    ASSERT_TRUE(msg.properties.correlationData.has_value());
    EXPECT_EQ(msg.properties.correlationData->size(), 3);
    EXPECT_EQ((*msg.properties.correlationData)[0], std::byte{0x01});

    ASSERT_TRUE(msg.properties.responseTopic.has_value());
    EXPECT_EQ(*msg.properties.responseTopic, "response/topic");
}

// Test PropertyUpdateResponse factory method
TEST(MqttMessageTest, PropertyUpdateResponseFactoryMethod) {
    std::vector<std::byte> correlationId = {std::byte{0xAA}, std::byte{0xBB}};

    auto msg = MqttMessage::PropertyUpdateResponse("prop/response", "success", 7, correlationId, 0, "Update completed");

    EXPECT_EQ(msg.topic, "prop/response");
    EXPECT_EQ(msg.payload, "success");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_FALSE(msg.retain);

    ASSERT_TRUE(msg.properties.propertyVersion.has_value());
    EXPECT_EQ(*msg.properties.propertyVersion, 7);

    ASSERT_TRUE(msg.properties.correlationData.has_value());
    EXPECT_EQ(msg.properties.correlationData->size(), 2);

    ASSERT_TRUE(msg.properties.returnCode.has_value());
    EXPECT_EQ(*msg.properties.returnCode, 0);

    ASSERT_TRUE(msg.properties.debugInfo.has_value());
    EXPECT_EQ(*msg.properties.debugInfo, "Update completed");
}

// Test MethodRequest factory method
TEST(MqttMessageTest, MethodRequestFactoryMethod) {
    std::vector<std::byte> correlationId = {std::byte{0x11}, std::byte{0x22}, std::byte{0x33}, std::byte{0x44}};

    auto msg = MqttMessage::MethodRequest("method/call", "{\"param\":\"value\"}", correlationId, "method/response");

    EXPECT_EQ(msg.topic, "method/call");
    EXPECT_EQ(msg.payload, "{\"param\":\"value\"}");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_FALSE(msg.retain);

    ASSERT_TRUE(msg.properties.correlationData.has_value());
    EXPECT_EQ(msg.properties.correlationData->size(), 4);

    ASSERT_TRUE(msg.properties.responseTopic.has_value());
    EXPECT_EQ(*msg.properties.responseTopic, "method/response");
}

// Test MethodResponse factory method
TEST(MqttMessageTest, MethodResponseFactoryMethod) {
    std::vector<std::byte> correlationId = {std::byte{0xFF}};

    auto msg = MqttMessage::MethodResponse("method/response", "{\"result\":42}", correlationId, 200, "OK");

    EXPECT_EQ(msg.topic, "method/response");
    EXPECT_EQ(msg.payload, "{\"result\":42}");
    EXPECT_EQ(msg.qos, 2);
    EXPECT_FALSE(msg.retain);

    ASSERT_TRUE(msg.properties.correlationData.has_value());
    EXPECT_EQ(msg.properties.correlationData->size(), 1);
    EXPECT_EQ((*msg.properties.correlationData)[0], std::byte{0xFF});

    ASSERT_TRUE(msg.properties.returnCode.has_value());
    EXPECT_EQ(*msg.properties.returnCode, 200);

    ASSERT_TRUE(msg.properties.debugInfo.has_value());
    EXPECT_EQ(*msg.properties.debugInfo, "OK");
}

// Test MqttProperties
TEST(MqttPropertiesTest, DefaultConstructor) {
    MqttProperties props;

    EXPECT_FALSE(props.correlationData.has_value());
    EXPECT_FALSE(props.responseTopic.has_value());
    EXPECT_FALSE(props.subscriptionId.has_value());
    EXPECT_FALSE(props.messageExpiryInterval.has_value());
    EXPECT_FALSE(props.contentType.has_value());
    EXPECT_FALSE(props.debugInfo.has_value());
    EXPECT_FALSE(props.returnCode.has_value());
    EXPECT_FALSE(props.propertyVersion.has_value());
    EXPECT_FALSE(props.version.has_value());
}

TEST(MqttPropertiesTest, SetProperties) {
    MqttProperties props;

    props.contentType = "application/json";
    props.messageExpiryInterval = 3600;
    props.returnCode = 0;
    props.version = "1.0.0";

    ASSERT_TRUE(props.contentType.has_value());
    EXPECT_EQ(*props.contentType, "application/json");

    ASSERT_TRUE(props.messageExpiryInterval.has_value());
    EXPECT_EQ(*props.messageExpiryInterval, 3600);

    ASSERT_TRUE(props.returnCode.has_value());
    EXPECT_EQ(*props.returnCode, 0);

    ASSERT_TRUE(props.version.has_value());
    EXPECT_EQ(*props.version, "1.0.0");
}

// Test edge cases
TEST(MqttMessageTest, EmptyPayload) {
    auto msg = MqttMessage::Signal("test/topic", "");

    EXPECT_EQ(msg.topic, "test/topic");
    EXPECT_EQ(msg.payload, "");
}

TEST(MqttMessageTest, EmptyCorrelationId) {
    std::vector<std::byte> emptyId;

    auto msg = MqttMessage::MethodRequest("test/topic", "payload", emptyId, "response");

    ASSERT_TRUE(msg.properties.correlationData.has_value());
    EXPECT_EQ(msg.properties.correlationData->size(), 0);
}
