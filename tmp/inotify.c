/* Playground for testing inotify.
 * Its purpose is to render dzenstat a bit more notification-based (instead of
 * polling - polling is bad).
 */

#include <stdlib.h>      /* select() stack */
#include <stdio.h>       /* printf() */
#include <unistd.h>      /* read(), close() */
#include <sys/inotify.h>
#include <string.h>      /* strcmp() */

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFLEN (1024*(EVENT_SIZE+16))
#define STDIN 0

int main()
{
	int fd, wd;
	int i, s, len;
	char buf[BUFLEN];
	struct inotify_event *ev;
	fd_set fds;

	/* initialise inotify */
	fd = inotify_init();
	if (fd < 0) {
		perror("inotify_init");
		return EXIT_FAILURE;
	}

	wd = inotify_add_watch(fd, "inotify.c", IN_MODIFY);
	if (wd < 0) {
		perror("inotify_add_watch");
		return EXIT_FAILURE;
	}

	while (1) {
		/* clear fd set */
		FD_ZERO(&fds);

		/* add our fd and STDIN to the list of fds to be observed */
		FD_SET(fd, &fds);
		FD_SET(STDIN, &fds);

		/* wait for activity */
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		printf("ACTIVITY!\n");
		if (s < 0) {
			fprintf(stderr, "select() < 0 (%d)\n", s);
			break;
		}

		/* check user input */
		if (FD_ISSET(STDIN, &fds)) {
			scanf("%s", buf);
			if (!strcmp(buf, "q"))
				break;
			printf("to exit, type 'q'\n");
		}
		
		/* process data */
		else {
			/* read data */
			len = read(fd, buf, BUFLEN);
			if (len < 0) {
				fprintf(stderr, "read() < 0\n");
				break;
			}

			/* print formatted data */
			i = 0;
			while (i < len) {
				printf("new event:\n");
				ev = (struct inotify_event *) &buf[i];
				printf("  wd=%d\n  mask=%u\n  cookie=%u\n  len=%u\n",
						ev->wd, ev->mask, ev->cookie, ev->len);
				if (ev->len)
					printf("  name=%s\n", ev->name);
				i += EVENT_SIZE + ev->len;
			}
		}
	}
	close(fd);

	return EXIT_SUCCESS;
}

