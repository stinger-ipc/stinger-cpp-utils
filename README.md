# Stinger C++ Utils

This provides common utilities needed for Stinger-generated C++ libraries.

## Features

## Requirements

- C++17 or later
- CMake 3.14 or later
- libmosquitto library

## Building

### Installing libmosquitto

On Ubuntu/Debian:
```bash
sudo apt-get install libmosquitto-dev
```

Or build from source:
```bash
git clone https://github.com/eclipse/mosquitto.git
cd mosquitto
mkdir build && cd build
cmake ..
make
sudo make install
```

### Build the Library

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Build Options

- `STINGER_UTILS_BUILD_TESTS` - Build tests (default: ON)
- `STINGER_UTILS_BUILD_EXAMPLES` - Build examples (default: ON)
- `STINGER_UTILS_BUILD_MOCK` - Build mock connection for testing (default: OFF)
- `BUILD_SHARED_LIBS` - Build shared library (default: OFF)

Example with custom options:
```bash
cmake -DSTINGER_UTILS_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON ..
```

To build with mock connection support (useful for unit testing):
```bash
cmake -DSTINGER_UTILS_BUILD_MOCK=ON ..
```

## Running Tests

```bash
cd build
ctest --output-on-failure
```

Or run the test executable directly:
```bash
./tests/stinger_utils_tests
```

## Running Examples

```bash
cd build
./examples/example_usage
```

## Installation

```bash
cd build
sudo cmake --install .
```

## Usage

After installation, you can use the library in your CMake project:

```cmake
find_package(StingerUtils REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE Stinger::Utils)
```

Example code:

```cpp
#include <stinger/utils/mqttbrokerconnection.hpp>
#include <stinger/utils/mqttmessage.hpp>

using namespace stinger::utils;

int main() {
    // Create MQTT connection
    auto mqtt = std::make_unique<MqttBrokerConnection>("localhost", 1883, "my_client");
    
    // Subscribe to a topic
    mqtt->Subscribe("sensor/temperature", 1);
    
    // Add message callback
    mqtt->AddMessageCallback([](const MqttMessage& msg) {
        std::cout << "Topic: " << msg.topic << ", Payload: " << msg.payload << std::endl;
    });
    
    // Publish a signal message
    auto msg = MqttMessage::Signal("sensor/temperature", "22.5");
    mqtt->Publish(msg);
    
    return 0;
}
```

## Project Structure

```
stinger-cpp-utils/
├── CMakeLists.txt          # Main build configuration
├── include/                # Public headers
│   └── stinger/utils/      # Namespace-aligned headers
├── src/                    # Implementation files
├── tests/                  # Unit tests (Google Test)
├── examples/               # Usage examples
├── cmake/                  # CMake configuration files
└── docs/                   # Documentation
```

## License

MIT License.

See LICENSE file for details.
