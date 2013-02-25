/* dzenstat configuration file
 */

// use design capacity for calculating percentage?
bool const use_acpi_real_capacity = false;

// delay in seconds for battery/temperature update:
int const update_interval = 10;

// location of battery information files (folder):
char const* battery_path = "/sys/class/power_supply/BAT1/";

// location of CPU temperature file:
char const* cpu_temperature_path = "/sys/class/hwmon/hwmon0/device/temp1_input";

// location of icon files:
char const* icons_path = "/home/ayekat/.config/conky/graphics/";

