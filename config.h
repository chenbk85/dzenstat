/* dzenstat configuration file
 */

// use design capacity for calculating percentage?
static bool const use_acpi_real_capacity = true;

// delay in seconds for battery/CPU update:
static int const update_interval = 1;

// location of battery information files (folder):
static char const* battery_path = "/sys/class/power_supply/BAT1";

// location of CPU temperature file:
static char const* cpu_temperature_paths[] = {
	"/sys/class/hwmon/hwmon0/device/temp1_input",
	"/sys/class/hwmon/hwmon0/temp1_input",
};

// number of CPU cores:
static int const num_cpus = 2;

// temperature thresholds (for highlighting with colours):
static int const temp_high = 85;
static int const temp_crit = 95;

// show network interfaces with IP even if they are down?
static bool const show_inactive_if = true;

// colours:
static char const* colour_light    = "555555"; // text colour in light area
static char const* colour_light_bg = "EEEEEE"; // background colour in light a.
static char const* colour_medium   = "EEEEEE"; // text colour in medium area
static char const* colour_medium_bg= "555555"; // background colour in medium a.
static char const* colour_hl       = "FFFFFF"; // highlighted text colour

static char const* colour_sep      = "555555"; // seperator colour
static char const* colour_err      = "FF3333"; // error strings
static char const* colour_warn     = "EEEE33"; // warning strings

