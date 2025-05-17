#include <stdio.h>
#include "taskLib.h"

void print(int n, char *text);

/*
 * Tasks
 * 1. Priorit�t einheitlich ver�ndern
 * -> 255 niedrigste, 0 h�chste
 * -> Ausgaben von print kommen eher gesammelt am Ende (bei CPU AUTO, also mehreren Kernen)
 * 2. print Argumente geben
 * 3. Argumente in task_spawn �bergeben
 * 4. Tasks verschiedene Priorit�ten zuweisen
 * -> die Reihenfolge �ndert sich jenach Priorit�t, Tasks mit h�herer Priorit�t als spawn_ten dr�ngeln sich vor
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
