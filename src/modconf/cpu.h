/* dzenstat CPU module configuration file
 */

static char const *path_load = "/proc/stat";            /* load */
static char const *paths_temp[] = {                     /* temperature */
	"/sys/class/hwmon/hwmon0/device/temp1_input",
	"/sys/class/hwmon/hwmon0/temp1_input",
};
static int const temp_high = 85;
static int const temp_crit = 95;

