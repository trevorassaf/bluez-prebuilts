cmake_minimum_required(VERSION 3.1)

include_directories(include)

add_executable(bt_scan src/bt_scan.cpp)
add_executable(bt_read src/bt_read.cpp)

file(GLOB BLUEZ_PREBUILT_LIBRARIES "lib/*.so*")
message("LIBRARIES = ${BLUEZ_PREBUILT_LIBRARIES}")
target_link_libraries(bt_scan ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_read ${BLUEZ_PREBUILT_LIBRARIES})

add_library(bluez SHARED IMPORTED)
set_property(TARGET bluez PROPERTY IMPORTED_LOCATION "lib/bluetooth.so")
