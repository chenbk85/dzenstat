/* dzenstat configuration file
 */

// use design capacity for calculating percentage?
static bool const use_acpi_real_capacity = false;

// delay in seconds for battery/CPU update:
static int const update_interval = 2;

// location of battery information files (folder):
static char const* battery_path = "/sys/class/power_supply/BAT1";

// location of CPU temperature file:
static char const* cpu_temperature_path = "/sys/class/hwmon/hwmon0/device/temp1_input";

// number of CPU cores:
static int const num_cpus = 2;

// temperature thresholds (for highlighting with colours):
static int const temp_high = 85;
static int const temp_crit = 95;

// colours:
static char const* colour_hl   = "FFFFFF"; // highlighted text
static char const* colour_hlbg = "444444"; // highlighted background
static char const* colour_sep  = "555555"; // seperator colour
static char const* colour_err  = "FF3333"; // error strings
static char const* colour_warn = "EEEE33"; // warning strings

