/* Example programme for detecting mouse/keyboard idle time.
 * Compile with -lXss
 */

#include <X11/extensions/scrnsaver.h>
#include <stdio.h>
#include <unistd.h>

int
main()
{
	XScreenSaverInfo *info = XScreenSaverAllocInfo();
	Display *display = XOpenDisplay(0);

	while (1) {
		XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
		printf("%lu ms\n", info->idle);
		usleep(1000000);
	}
}

