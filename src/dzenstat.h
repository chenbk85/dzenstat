/* Header file for dzenstat.
 */

#ifndef DZENSTAT_H
#define DZENSTAT_H

#include <stdbool.h>

#define BUFLEN 128      /* length for buffers */
#define DISPLEN 512     /* length for display buffers (a little longer) */

/* Used to describe a module.
 */
typedef struct Module {
	int (*init)(struct Module *mod);     /* init function */
	int (*update)(void);                 /* periodic update */
	int (*interrupt)(void);              /* event handler */
	int (*term)(void);                   /* cleanup procedure */
	bool ignore;                         /* no periodic updates? */
	bool has_fd;                         /* listen to events? */
	int fd;                              /* ... if yes, file descriptor */
	char display[DISPLEN];               /* module display */
	bool stumbled;                       /* failed to initialise? */
} Module;

/* Returns a colour depending on the value from red (0) to green (100).
 */
unsigned int colour(int val);

/* Writes a message to stderr.
 */
void wrlog(char const *format, ...);

#endif

