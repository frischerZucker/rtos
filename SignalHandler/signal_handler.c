/* includes */
#include "vxWorks.h"
#include "sigLib.h"
#include "taskLib.h"
#include "stdio.h"

/*
 * Signal Handler
 * 1. Programm zum Laufen bekommen
 * -> LONG_TIME vergrößern (glaube x20)
 * -> Prioritäten nach abgeschlossenen Aktionen vertauschen, damit der andere Task laufen kann
 * 2. kein Signal Handler für sigCatcher installieren (#define AUFGABE_2 aktivieren)
 * -> Ausgabe wird zu "+++++++++++++++++++++++++++++++SIGINT signal generated
 *					   Normal processing in sigCatcher
 *					   +++++++++++++++++++++++++++++++SIGINT signal generated
 *					   Normal processing in sigCatcher"
 * -> Signal läuft ins Nichts, hat keine Auswirkung
 * 3. SIGINT blockieren (#define AUFGABE_3 aktivieren)
 * -> gleiche Ausgabe wie in 2.
 */
//#define AUFGABE_2
//#define AUFGABE_3

/* function prototypes */
void catchSIGINT(int);
void sigCatcher(void);

/* globals */
#define NO_OPTIONS 0
#define ITER1 100
#define LONG_TIME 20000000
#define HIGH_PRIORITY 90
#define LOW_PRIORITY 110
TASK_ID tid_generator, tid_sig_catcher;

void sigGenerator(void) /* task to generate the SIGINT signal */
{
	STATUS taskAlive;
	
	if((tid_sig_catcher = taskSpawn("signal",100,0x100,20000,(FUNCPTR)sigCatcher,0,0,0,0,0,0,0,0,0,0)) == ERROR)
		printf("taskSpawn sigCatcher failed\n");
	
	tid_generator = taskIdSelf(); /* get sigGenerator's task id */
	
	taskDelay(30); /* allow time to get sigCatcher to run */
	
	for (int i=0; i < ITER1; i++)
	{
		if ((taskAlive = taskIdVerify(tid_sig_catcher)) == OK)
		{
			printf("+++++++++++++++++++++++++++++++SIGINT signal generated\n");
			kill(tid_sig_catcher, SIGINT); /* generate signal */
			/* lower sigGenerator priority to allow sigCatcher to run */
			taskPrioritySet(tid_sig_catcher, HIGH_PRIORITY);
			taskPrioritySet(tid_generator,LOW_PRIORITY);
		}
		else  /* sigCatcher is dead */
			break;
	}
	printf("\n***************sigGenerator Exited***************\n");
}

void sigCatcher(void) /* task to handle the SIGINT signal */
{
	struct sigaction newAction;
	
	newAction.sa_handler = catchSIGINT; /* set the new handler */
	sigemptyset(&newAction.sa_mask); /* no other signals blocked */
	newAction.sa_flags = NO_OPTIONS;	/* no special options */
#ifdef AUFGABE_3 // SIGINT blockieren
	sigblock(SIGMASK(SIGINT));
#endif // AUFGABE_3
	
#ifndef AUFGABE_2 // kein Signal Handler installieren
	if(sigaction(SIGINT, &newAction, NULL) == -1)
		printf("Could not install signal handler\n");
#endif // AUFGABE_2
	
	for (int i=0; i < ITER1; i++)
	{
		for (int  j=0; j < LONG_TIME; j++);
		
		printf("Normal processing in sigCatcher\n");
		taskPrioritySet(tid_generator, HIGH_PRIORITY);
		taskPrioritySet(tid_sig_catcher, LOW_PRIORITY);
	}
	
	printf("\n+++++++++++++++sigCatcher Exited+++++++++++++++\n");
}

void catchSIGINT(int signal)  /* signal handler code */
{
	printf("-------------------------------SIGINT signal caught\n");
	/* increase sigGenerator priority to allow sigGenerator to run */
	taskPrioritySet(tid_generator,HIGH_PRIORITY);
	taskPrioritySet(tid_sig_catcher, LOW_PRIORITY);
}
