/* gcc -pthread */

#include <stdio.h>
#include <pthread.h>

int c;

void *
mythread(void *args)
{
	int i;

	for (i = 0; i < 10; i++, c--)
		printf("\033[1;31m-\033[0m\n");
	return NULL;
}

int
main(int argc, char **argv)
{
	int i;
	pthread_t t;

	c = 0;
	pthread_create(&t, NULL, mythread, NULL);

	for (i = 0; i < 10; i++, c++)
		printf("\033[1;32m+\033[0m\n");
	
	pthread_join(t, NULL);
	printf("c = %d\n", c);
	return 0;
}

