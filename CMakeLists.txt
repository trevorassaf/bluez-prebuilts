cmake_minimum_required(VERSION 3.1)

include_directories(
  include/bluez-5.50
  include/bluez-5.50/lib
  src/gdbus
  src/client
  ../../glib-prebuilts/repo/include
  ../../glib-prebuilts/repo/include/glib
  ../../glib-prebuilts/repo/include/gio
  ../../glib-prebuilts/repo/include/gmodule
  ../../glib-prebuilts/repo/include/gobject
  ../../dbus-prebuilts/repo/include/dbus-1.12.20)

add_executable(bt_scan src/bt_scan.cpp)
add_executable(bt_read src/bt_read.cpp)
add_executable(bt_get_managed_objects
  src/bt_get_managed_objects.cpp
  src/gdbus/client.c
  src/gdbus/mainloop.c
  src/gdbus/object.c
  src/gdbus/polkit.c
  src/gdbus/watch.c)

add_executable(bt_test_gdbus
  src/bt_test_gdbus.cpp
  src/gdbus/client.c
  src/gdbus/mainloop.c
  src/gdbus/object.c
  src/gdbus/polkit.c
  src/gdbus/watch.c)

add_executable(bt_connect
  src/bt_connect.cpp
  src/agent.c
  src/gdbus/client.c
  src/gdbus/mainloop.c
  src/gdbus/object.c
  src/gdbus/polkit.c
  src/gdbus/watch.c)

add_executable(bt_hci_scan
  src/bt_hci_scan.cpp
  src/agent.c
  src/gdbus/client.c
  src/gdbus/mainloop.c
  src/gdbus/object.c
  src/gdbus/polkit.c
  src/gdbus/watch.c)

add_executable(bt_sdp_query
  src/bt_sdp_query.cpp
  src/agent.c
  src/gdbus/client.c
  src/gdbus/mainloop.c
  src/gdbus/object.c
  src/gdbus/polkit.c
  src/gdbus/watch.c)

add_executable(bt_xbox_controller_read
  src/bt_xbox_controller_read.cpp)

file(GLOB BLUEZ_PREBUILT_LIBRARIES "lib/*.so*")
message("BLUEZ_LIBRARIES = ${BLUEZ_PREBUILT_LIBRARIES}")

file(GLOB DBUS_PREBUILT_LIBRARIES "../../dbus-prebuilts/repo/lib/*.so*")
message("DBUS_LIBRARIES = ${DBUS_PREBUILT_LIBRARIES}")

file(GLOB GLIB_PREBUILT_LIBRARIES "../../glib-prebuilts/repo/lib/*.so*")
message("GLIB_LIBRARIES = ${GLIB_PREBUILT_LIBRARIES}")

target_link_libraries(bt_get_managed_objects ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_get_managed_objects ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_get_managed_objects ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_test_gdbus ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_test_gdbus ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_test_gdbus ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_connect ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_connect ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_connect ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_hci_scan ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_hci_scan ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_hci_scan ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_sdp_query ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_sdp_query ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_sdp_query ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_xbox_controller_read ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_xbox_controller_read ${DBUS_PREBUILT_LIBRARIES})
target_link_libraries(bt_xbox_controller_read ${GLIB_PREBUILT_LIBRARIES})

target_link_libraries(bt_scan ${BLUEZ_PREBUILT_LIBRARIES})
target_link_libraries(bt_read ${BLUEZ_PREBUILT_LIBRARIES})

add_library(bluez SHARED IMPORTED)
set_property(TARGET bluez PROPERTY IMPORTED_LOCATION "lib/bluetooth.so")
