/* dzenstat configuration file
 */

/* SYSTEM ----------------------------------------------------------------------
 * These variables determine where to look for system information and how to
 * handle them.
 */

// paths:
static char const *path_bat = "/sys/class/power_supply/BAT1"; // battery
static char const *path_cpu_load = "/proc/stat";              // CPU load
static char const *path_cpu_temp[] = {                        // CPU temperature
	"/sys/class/hwmon/hwmon0/device/temp1_input",
	"/sys/class/hwmon/hwmon0/temp1_input",
};
static char const *path_mem = "/proc/meminfo";                // memory (RAM)
static char const *path_snd = "/proc/asound/SB/codec#0";      // sound


/* DZENSTAT SETTINGS -----------------------------------------------------------
 * These variables control the behaviour of dzenstat.
 * (I'm sorry, for now there isn't much to configure, but it will get extended)
 */

// use design capacity for calculating percentage?
static bool const use_acpi_real_capacity = true;

// delay in seconds for battery/CPU update:
static int const update_interval = 2;

// show network interfaces with IP even if they are down?
static bool const show_inactive_if = true;

// temperature thresholds (for highlighting with colours):
static int const temp_high = 85;
static int const temp_crit = 95;


/* DZENSTAT LOOK ---------------------------------------------------------------
 * These variables control the look of dzenstat.
 * (colours, icons, ...)
 */

static char const *path_icons = "icons";     // path to icons folder

// colours:
static int const colour_light     = 0x555555; // light area text colour
static int const colour_light_bg  = 0xEEEEEE; // light area background colour
static int const colour_medium    = 0xEEEEEE; // medium area text colour
static int const colour_medium_bg = 0x555555; // medium area background colour

static int const colour_hl        = 0xFFFFFF; // highlighted text colour
static int const colour_ok        = 0x33EE33; // success text colour ('green')
static int const colour_warn      = 0xEEEE33; // warning text colour ('yellow')
static int const colour_err       = 0xEE3333; // error text colour   ('red')

static int const colour_sep       = 0x555555; // seperator colour
static int const colour_bat       = 0x4499CC; // battery colour if being charged

