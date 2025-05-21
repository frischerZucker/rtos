/* includes */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "vxWorks.h"

/*
 * Uebung 1: Threadterminierung
 * - drei Threads erzeugen
 * - einer erzeugt Zufallszahl z (0 <= z <= 50), andere reagieren darauf
 * - wenn  z < 25 beendet der eine den anderen, wenn z > 25 anders herum
 *   (mit pthread_cancel)
 */

#define TID_ZUFALLSZAHL 0
#define TID_KLEINER 1
#define TID_GROESSER 2

void *thread_kleiner25(void *threadid);
void *thread_groesser25(void *threadid);
void *thread_zufallszahl(void *args);

pthread_t threads[3];
char z = -1;
char threads_fertig = 0;

void start(void) {}

int main(int argc, char *argv[])
{
	int rc;
	
	printf("\n\n");
	
	rc = pthread_create(&threads[TID_ZUFALLSZAHL], NULL, thread_zufallszahl, (void *) TID_ZUFALLSZAHL);
	if(rc)
	{
		printf("ERROR: return code from pthread_create ist %d\n", rc);
		exit(-1);
	}
		
	rc = pthread_create(&threads[TID_KLEINER], NULL, thread_kleiner25, (void *) TID_KLEINER);
	if(rc)
	{
		printf("ERROR: return code from pthread_create ist %d\n", rc);
		exit(-1);
	}
		
	rc = pthread_create(&threads[TID_GROESSER], NULL, thread_groesser25, (void *) TID_GROESSER);
	if(rc)
		{
		printf("ERROR: return code from pthread_create ist %d\n", rc);
		exit(-1);
	}

	return 0;
}

/*
 * Erzeugt eine Zufallszahl zwischen 1 und 50. Speichert diese in z.
 */
void *thread_zufallszahl(void *threadid)
{
	int tid = (int)threadid;
	
	printf("Thread %d erzeugt.\n", tid);
	
	srand(time(NULL));
	
	z = rand() % 50 + 1;
	
	printf("z = %d\n", z);
	
	pthread_exit(NULL);
	return 0;
}

/*
 * Wartet bis z eine Zufallszahl zugewiesen wurde und beendet dann den Thread TID_GROESSER, falls z <= 25.
 */
void *thread_kleiner25(void *threadid)
{
	int tid = (int)threadid;
	
	printf("Thread %d erzeugt.\n", tid);
	
	// Gibt dem anderen Thread Zeit um erzeugt zu werden.
	for(int i = 0; i < 3; i++) 
	{
		printf("Thread %d wartet.. i=%d\n", tid, i);
		taskDelay(100);
	}
	
	// Wartet bis Zufallszahl erzeugt ist.
	while(z == -1);
	
	while(1)
	{
		if(z <= 25)
		{
			pthread_cancel(threads[TID_GROESSER]);
			printf("z <= 25. Thread %d gecancelt.\n", TID_GROESSER);
			for(int i = 0; i < 3; i++) 
				{
					printf("Thread %d laeuft noch.. i=%d\n", tid, i);
					taskDelay(10);
				}
			pthread_exit(NULL);
		}
	}
	
	return 0;
}

/*
 * Wartet bis z eine Zufallszahl zugewiesen wurde und beendet dann den Thread TID_KLEINER, falls z > 25.
 */
void *thread_groesser25(void *threadid)
{
	int tid = (int)threadid;
	
	printf("Thread %d erzeugt.\n", tid);

	// Gibt dem anderen Thread Zeit um erzeugt zu werden.
	for(int i = 0; i < 3; i++) 
	{
		printf("Thread %d wartet.. i=%d\n", tid, i);
		taskDelay(100);
	}
	
	// Wartet bis Zufallszahl erzeugt ist.
	while(z == -1);
	
	while(1)
	{
		if(z > 25)
		{
			pthread_cancel(threads[TID_KLEINER]);
			printf("z > 25. Thread %d gecancelt.\n", TID_KLEINER);
			for(int i = 0; i < 3; i++) 
				{
					printf("Thread %d laeuft noch.. i=%d\n", tid, i);
					taskDelay(10);
				}
			pthread_exit(NULL);
		}
	}
	
	return 0;
}
