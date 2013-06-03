/* Example of how to use fork().
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int
main(int argc, char **argv)
{
	int i;
	pid_t pid = fork();
	if (pid < 0)
		printf("%s\n", strerror(errno));

	for (i = 0; i < 10; i++)
		printf("\e[%dm%s:%d\e[0m\n",
				pid == 0 ? 32 : 33, pid == 0 ? "new" : "old", i);

	/* fork() is handy in combination with exec:
	if (pid == 0)
		return execlp("ls", "ls", (char *) NULL);
	*/
}

