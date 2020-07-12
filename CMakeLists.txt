cmake_minimum_required(VERSION 3.1)

include_directories(include)

add_executable(test_bluez test_bluez.cpp)

file(GLOB BLUEZ_PREBUILT_LIBRARIES "lib/*.so*")
message("LIBRARIES = ${BLUEZ_PREBUILT_LIBRARIES}")
target_link_libraries(test_bluez ${BLUEZ_PREBUILT_LIBRARIES})

add_library(bluez SHARED IMPORTED)
set_property(TARGET bluez PROPERTY IMPORTED_LOCATION "lib/bluetooth.so")
