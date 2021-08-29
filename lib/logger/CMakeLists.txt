add_library(hal-interfaces-logger INTERFACE)
add_library(hal::interfaces-logger ALIAS hal-interfaces-logger)

target_include_directories(hal-interfaces-logger
    INTERFACE include
)

target_link_libraries(hal-interfaces-logger
    INTERFACE utils::logger
)
