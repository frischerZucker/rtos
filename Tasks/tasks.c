#include <stdio.h>
#include "taskLib.h"

void print(int n, char *text);

/*
 * Tasks
 * 1. Priorität einheitlich verändern
 * -> 255 niedrigste, 0 höchste
 * -> Ausgaben von print kommen eher gesammelt am Ende (bei CPU AUTO, also mehreren Kernen)
 * 2. print Argumente geben
 * 3. Argumente in task_spawn übergeben
 * 4. Tasks verschiedene Prioritäten zuweisen
 * -> die Reihenfolge ändert sich jenach Priorität, Tasks mit höherer Priorität als spawn_ten drängeln sich vor
 */

void spawn_ten()
{
	int i;
	TASK_ID taskId;
	
	printf("\n\n");
	
	srand(time(NULL));
	
	for (i = 0; i < 10; ++i) {
		int priority = rand() % 255;
		
		printf("spawn task %d with priority %d\n", i, priority);
		
		taskId = taskSpawn("Print", priority, 0x100, 2000, (FUNCPTR) print, i, "test", 0, 0, 0, 0, 0, 0, 0, 0);
	}
}

void print(int n, char *text)
{
	int priority;
	taskPriorityGet(taskIdSelf(), &priority);
	printf("Hello, I am task %d (%d) with priority %d: %s\n", taskIdSelf(), n, priority, text);
}
