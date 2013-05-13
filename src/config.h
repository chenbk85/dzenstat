/* dzenstat main configuration file
 */

#ifndef DZENSTAT_CONFIG_H
#define DZENSTAT_CONFIG_H

/* SYSTEM ----------------------------------------------------------------------
 * These variables determine where to look for system information and how to
 * handle them.
 */

/* paths */
static char const *path_bat = "/sys/class/power_supply/BAT1"; /* battery */
static char const *path_mem = "/proc/meminfo";                /* memory (RAM) */


/* MODULES ---------------------------------------------------------------------
 * Include modules to be loaded and define the order in which they shall appear
 * in the bar.
 */

#include "cpu.h"

static Module modules[] = {
	{ .init = cpu_init },
};


/* DZENSTAT SETTINGS -----------------------------------------------------------
 * These variables control the behaviour of dzenstat.
 * (I'm sorry, for now there isn't much to configure, but it will get extended)
 */

/* use design capacity for calculating percentage? */
static bool const use_acpi_real_capacity = true;

/* delay in seconds for battery/CPU update */
#define update_interval 2

/* show network interfaces with IP even if they are down? */
static bool const show_inactive_if = true;


/* DZENSTAT LOOK ---------------------------------------------------------------
 * These variables control the look of dzenstat.
 * (colours, icons, ...)
 */

#define path_icons "icons"      /* path to icons folder */

/* colours */
#define colour_light     0x555555 /* light area fg */
#define colour_light_bg  0xEEEEEE /* light area bg */
#define colour_medium    0xEEEEEE /* medium area fg */
#define colour_medium_bg 0x555555 /* medium area bg */

#define colour_hl        0xFFFFFF
#define colour_ok        0x33EE33 /* success fg (green) */
#define colour_warn      0xEEEE33 /* warning fg (yellow) */
#define colour_err       0xEE3333 /* error fg (red) */

#define colour_sep       0x555555 /* seperator */
#define colour_bat       0x4499CC /* battery if charging */

#endif

