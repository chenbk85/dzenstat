/* dzenstat global configuration file
 */

#ifndef DZENSTAT_COLOURS_H
#define DZENSTAT_COLOURS_H

/* interval for polling */

#define update_interval 2

/* icons location */

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

