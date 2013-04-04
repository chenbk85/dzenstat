/* dzenstat configuration file
 */

/* SYSTEM ----------------------------------------------------------------------
 * These variables determine where to look for system information and how to
 * handle them.
 */

static char const *battery_path = "/sys/class/power_supply/BAT1";  // battery
static char const *cpu_usage_path = "/proc/stat";  // CPU usage
static char const *cpu_temperature_paths[] = {     // CPU temperature
	"/sys/class/hwmon/hwmon0/device/temp1_input",
	"/sys/class/hwmon/hwmon0/temp1_input",
};
static char const *memory_path = "/proc/meminfo";  // RAM

// temperature thresholds (for highlighting with colours):
static int const temp_high = 85;
static int const temp_crit = 95;


/* DZENSTAT SETTINGS -----------------------------------------------------------
 * These variables control the behaviour of dzenstat.
 * (I'm sorry, for now there isn't much to configure, but it will get extended)
 */

// use design capacity for calculating percentage?
static bool const use_acpi_real_capacity = true;

// delay in seconds for battery/CPU update:
static int const update_interval = 3;

// show network interfaces with IP even if they are down?
static bool const show_inactive_if = true;

// where are the icons stored in?
static char const *icons_path = "icons";

// colours:
static char const* colour_light    = "555555"; // text colour in light area
static char const* colour_light_bg = "EEEEEE"; // background colour in light a.
static char const* colour_medium   = "EEEEEE"; // text colour in medium area
static char const* colour_medium_bg= "555555"; // background colour in medium a.
static char const* colour_hl       = "FFFFFF"; // highlighted text colour

static char const* colour_sep      = "555555"; // seperator colour
static char const* colour_err      = "FF3333"; // error strings
static char const* colour_warn     = "EEEE33"; // warning strings

