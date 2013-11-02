#include <dbus/dbus.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	DBusConnection *conn;
	DBusError err;
	int ret;

	/* initialise error handler */
	dbus_error_init(&err);

	/* connect to the bus */
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection error (%s)\n", err.message);
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}
	if (conn == NULL) {
		fprintf(stderr, "conn == NULL\n");
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}
	printf("Connection to DBUS established\n");

	/* request name on the bus */
	ret = dbus_bus_request_name(conn, "ch.ayekat.dzenstat.dbus",
			DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Name error (%s)\n", err.message);
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		fprintf(stderr, "Not primary owner\n");
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}
	printf("Name requested\n");

	/* close connection and free error handler */
	dbus_error_free(&err);
	dbus_connection_close(conn);
}

