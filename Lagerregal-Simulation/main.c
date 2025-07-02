/* includes */

#include <stdio.h>
#include <timexLib.h>

#include "msgQLib.h"
#include "taskLib.h"
#include "vxWorks.h"

#include "auftragsverwaltung.h"
#include "data_types.h"

#define MAX_AUFTRAEGE 10
#define MAX_AUFTRAGS_MSG_LENGTH 100
#define MAX_TARGET_POS 1

#define MS_TO_NS 1000*1000

MSG_Q_ID msgq_auftraege, msgq_target_pos;

timer_t timer_id;
struct itimerspec timer_spec;

const vec3_t POS_EINGABE = {10, 1, Z_EA};
const vec3_t POS_AUSGABE = {10, 1, Z_EA};

int start(void) {
	init_auftragsverwaltung();
	
	if ((msgq_auftraege = msgQCreate(MAX_AUFTRAEGE, MAX_AUFTRAGS_MSG_LENGTH, MSG_Q_FIFO)) == NULL)
	{
		printf("ERROR: msgQCreate for msgq_auftraege failed!\n");
	}
	if ((msgq_target_pos = msgQCreate(MAX_TARGET_POS, MAX_AUFTRAGS_MSG_LENGTH, MSG_Q_FIFO)) == NULL)
	{
		printf("ERROR: msgQCreate for msgq_target_pos failed!\n");
	}
	
	timer_spec.it_value.tv_sec = 1;
	timer_spec.it_value.tv_nsec = 0;
	timer_spec.it_interval.tv_sec = 1;
	timer_spec.it_interval.tv_nsec = 0;
	if (timer_create(CLOCK_REALTIME, NULL, &timer_id) == ERROR)
	{
		printf("ERROR: Creating timer failed!");
		return(errno);
	}
	// timer_callback mit Timer verbinden
	if (timer_connect(timer_id, (VOIDFUNCPTR) auftragsverwaltung, NULL) == ERROR)
	{
		printf("ERROR: Connecting calc_distance to the timer failed!");
		return(errno);
	}
	// Timer starten
	if (timer_settime(timer_id, TIMER_RELTIME, &timer_spec, NULL) == ERROR)
	{
		printf("ERROR: timer_settime failed!");
		return(errno);
	}
	
	auftrag_t test = {AUSLAGERN, WARTET, {4, 2}};

	taskDelay(100);
	
	if((msgQSend(msgq_auftraege, (char*)(void*)&test, MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
	{
		printf("ERROR: msgQSend for msgq_auftraege failed!\n");
	}
	
	taskDelay(10);
	
	//TASK_ID t = taskSpawn("auftragsverwaltung", 101, 0x100, 2000, (FUNCPTR) auftragsverwaltung, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0);
	while(1);
}
