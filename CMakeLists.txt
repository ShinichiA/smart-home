cmake_minimum_required(VERSION 3.14)
project(iot_smart_home VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings (senior-level strict)
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wnon-virtual-dtor
    -Wold-style-cast -Wcast-align
    -Woverloaded-virtual -Wconversion
    -Wsign-conversion -Wnull-dereference
    -Wformat=2
)

# Source files by module
set(CORE_SOURCES
    src/core/Logger.cpp
    src/core/ConfigManager.cpp
    src/core/EventBus.cpp
)

set(SENSOR_SOURCES
    src/sensors/BaseSensor.cpp
    src/sensors/TemperatureSensor.cpp
    src/sensors/HumiditySensor.cpp
    src/sensors/MotionSensor.cpp
    src/sensors/SensorFactory.cpp
)

set(COMMUNICATION_SOURCES
    src/communication/MqttClient.cpp
    src/communication/HttpClient.cpp
    src/communication/ProtocolAdapter.cpp
    src/communication/CommunicationFactory.cpp
)

set(PIPELINE_SOURCES
    src/pipeline/DataValidator.cpp
    src/pipeline/DataFilter.cpp
    src/pipeline/DataTransformer.cpp
    src/pipeline/DataPipeline.cpp
)

set(DEVICE_SOURCES
    src/devices/DeviceState.cpp
    src/devices/DeviceCommand.cpp
    src/devices/DeviceController.cpp
)

set(SERVICE_SOURCES
    src/services/SensorService.cpp
    src/services/CommunicationService.cpp
    src/services/AutomationService.cpp
)

set(APP_SOURCES
    src/app/Application.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${SENSOR_SOURCES}
    ${COMMUNICATION_SOURCES}
    ${PIPELINE_SOURCES}
    ${DEVICE_SOURCES}
    ${SERVICE_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# Copy config to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/config/config.json
    ${CMAKE_BINARY_DIR}/config/config.json
    COPYONLY
)
