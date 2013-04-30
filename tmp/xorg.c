/* A test programme for writing an X application.
 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	Display *dpy;
	int black, white;
	Window w;
	GC gc;
	XEvent e;

	/* get access to the (default) X display */
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "failed to open display\n");
		return EXIT_FAILURE;
	}

	/* define some colours */
	black = BlackPixel(dpy, DefaultScreen(dpy));
	white = WhitePixel(dpy, DefaultScreen(dpy));

	/* create window */
	w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200, 100, 3,
	                        white, black);
	
	/* mask notifications we want to retrieve by XNextEvent() (see below) */
	XSelectInput(dpy, w, ButtonPressMask|StructureNotifyMask);

	/* map window to display (= make it appear) */
	XMapWindow(dpy, w);

	/* create graphics context, to communicate graphical stuff to the server */
	gc = XCreateGC(dpy, w, 0, NULL);

	/* set foreground colour */
	XSetForeground(dpy, gc, white);
	
	/* wait for MapNotify event (for the three lines above) */
	for (; e.type != MapNotify; XNextEvent(dpy, &e));

	/* draw the line and flush */
	XDrawLine(dpy, w, gc, 10, 60, 180, 20);
	XFlush(dpy);

	/* wait for button press, then close */
	while (1) {
		XNextEvent(dpy, &e);
		switch (e.type) {
			case Expose:
				printf("Expose\n");
				break;
			case ButtonPress:
				printf("ButtonPress\n");
				XCloseDisplay(dpy);
				return EXIT_SUCCESS;
		}
	}
}

