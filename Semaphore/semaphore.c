/* includes */
#include "vxWorks.h"
#include "taskLib.h"
#include "semLib.h"
#include "stdio.h"


/*
 * Semaphore
 * 1. Programm zum Laufen bringen
 * 2. Note 1, Note 2 entfernen
 * -> Task 1 zählt erst bis 10 hoch, danach zählt Task 2 wieder bis 0 runter
 * 	Note1, Note2 sorgen dafür, dass beide Tasks erst einmal leben, bevor das zählen beginnt.
 *  Da die Tasks eine höhere Priorität als binary() haben, läuft ansonsten erst Task1 durch, bevor Task2 erzeugt wird.
 * 3. mit Zählsemaphor realisieren
 */
//#define AUFGABE_2
#define AUFGABE_3

/* function prototypes */
void taskOne(void);
void taskTwo(void);
void count_up();
void count_down();

/* globals */
#define ITER 10
#define INITIAL_SEM_COUNT 1
SEM_ID sem_binary, sem_counting;
int global = 0;

#ifndef AUFGABE_3
/*
 * Code für Aufgabe 1 + 2
 */
void binary(void)
{
	TASK_ID taskIdOne, taskIdTwo;
	
	/* create semaphore with semaphore available and queue tasks on FIFO basis */
	sem_binary = semBCreate(SEM_Q_FIFO, SEM_FULL);
	
	/* Note 1: lock the semaphore for scheduling purposes */
#ifndef AUFGABE_2
	semTake(sem_binary,WAIT_FOREVER);
#endif // AUFGABE_2
	
	/* spawn the two tasks */
	taskIdOne = taskSpawn("t1",90,0x100,2000,(FUNCPTR)taskOne,0,0,0,0,0,0,0,0,0,0);
	taskIdTwo = taskSpawn("t2",90,0x100,2000,(FUNCPTR)taskTwo,0,0,0,0,0,0,0,0,0,0);
}


void taskOne(void)
{
	int i;
	for (i=0; i < ITER; i++)
	{
		semTake(sem_binary,WAIT_FOREVER); /* wait indefinitely for semaphore */
		printf("I am taskOne and global = %d......................\n", ++global);
		semGive(sem_binary); /* give up semaphore */
	} 
}

void taskTwo(void)
{
	int i;
#ifndef AUFGABE_2
	semGive(sem_binary); /* Note 2: give up semaphore(a scheduling fix) */
#endif // AUFGABE_2
	for (i=0; i < ITER; i++)
	{
		semTake(sem_binary,WAIT_FOREVER); /* wait indefinitely for semaphore */
		printf("I am taskTwo and global = %d----------------------\n", --global);
		semGive(sem_binary); /* give up semaphore */
	} 
}
#else
/*
 * Code für Aufgabe 3
 */
void counting(void)
{
	sem_counting = semCCreate(SEM_Q_FIFO, INITIAL_SEM_COUNT);
	semCTake(sem_counting, WAIT_FOREVER);
	
	taskSpawn("up",90,0x100,2000,(FUNCPTR)count_up,0,0,0,0,0,0,0,0,0,0);
	taskSpawn("down",90,0x100,2000,(FUNCPTR)count_down,0,0,0,0,0,0,0,0,0,0);
}

void count_up()
{
	for (int i = 0; i < ITER; i++)
	{
		semCTake(sem_counting, WAIT_FOREVER);
		
		global += 1;
		printf("I am task1 and global = %d......................\n", global);
		
		semCGive(sem_counting);
	}
}

void count_down()
{
	semCGive(sem_counting);
	for (int i = 0; i < ITER; i++)
	{
		semCTake(sem_counting, WAIT_FOREVER);
		
		global -= 1;
		printf("I am task2 and global = %d......................\n", global);
		
		semCGive(sem_counting);
	}
}
#endif // AUFGABE_3
