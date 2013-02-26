/* dzenstat configuration file
 */

// use design capacity for calculating percentage?
static bool const use_acpi_real_capacity = false;

// delay in seconds for battery/CPU update:
static int const update_interval = 3;

// location of battery information files (folder):
static char const* battery_path = "/sys/class/power_supply/BAT1";

// location of CPU temperature file:
static char const* cpu_temperature_path = "/sys/class/hwmon/hwmon0/device/temp1_input";

// location of icon files:
static char const* icons_path = "/home/ayekat/.config/conky/graphics";

// number of CPU cores:
static int num_cpus = 2;

// colours:
static char const* colour_hl   = "FFFFFF"; // highlighted text
static char const* colour_hlbg = "444444"; // highlighted background

