cmake_minimum_required(VERSION 3.24)

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++20> $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)

add_subdirectory(logger)

add_library(hal-interfaces EXCLUDE_FROM_ALL
    Device.cpp
    Error.cpp
    IEeprom.cpp
    IHumiditySensor.cpp
    II2c.cpp
    IRtc.cpp
    ISpi.cpp
    ITemperatureSensor.cpp
    IUart.cpp
)
add_library(hal::interfaces ALIAS hal-interfaces)

target_include_directories(hal-interfaces
    PUBLIC include
)

target_link_libraries(hal-interfaces
    PUBLIC osal::cpp utils::types utils::registry
    PRIVATE hal::interfaces-logger
)
