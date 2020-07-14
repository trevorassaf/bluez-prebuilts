cmake_minimum_required(VERSION 3.1)

include_directories(
  include/bluez-5.50
  ../../glib-prebuilts/repo/include
  ../../glib-prebuilts/repo/include/glib
  ../../glib-prebuilts/repo/include/gio
  ../../glib-prebuilts/repo/include/gmodule
  ../../glib-prebuilts/repo/include/gobject)

add_executable(bt_scan src/bt_scan.cpp)
add_executable(bt_read src/bt_read.cpp)
add_executable(bt_test_gdbus src/bt_test_gdbus.cpp)

file(GLOB BLUEZ_PREBUILT_LIBRARIES "lib/*.so*")
message("LIBRARIES = ${BLUEZ_PREBUILT_LIBRARIES}")
target_link_libraries(bt_scan ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_read ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_test_gdbus ${BLUEZ_PREBUILT_LIBRARIES})

add_library(bluez SHARED IMPORTED)
set_property(TARGET bluez PROPERTY IMPORTED_LOCATION "lib/bluetooth.so")
