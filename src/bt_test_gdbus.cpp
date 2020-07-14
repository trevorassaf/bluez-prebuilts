#include <cstdlib>
#include <iostream>

#include <glib.h>
#include <gdbus/gdbus.h>

int main(int argc, char** argv)
{

  DBusConnection *dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
	g_dbus_attach_object_manager(dbus_conn);

	GDBusClient *client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_unref(client);

	dbus_connection_unref(dbus_conn);

  return EXIT_SUCCESS;
}
