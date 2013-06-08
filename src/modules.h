/* dzenstat main configuration file
 */

#ifndef DZENSTAT_CONFIG_H
#define DZENSTAT_CONFIG_H


/* MODULES ---------------------------------------------------------------------
 * Include modules to be loaded and define the order in which they shall appear
 * in the bar.
 */

#include "memory.h"
#include "cpu.h"
#include "network.h"
#include "battery.h"
#include "sound.h"
#include "mpd.h"
#include "date.h"

static Module modules[] = {
	{ .init = memory_init },
	{ .init = cpu_init },
	{ .init = network_init },
	{ .init = mpd_init },
	{ .init = battery_init },
	{ .init = sound_init },
	{ .init = date_init },
};

#endif

