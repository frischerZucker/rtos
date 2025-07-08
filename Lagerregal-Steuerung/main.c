/* includes */

#include <stdio.h>
#include <timexLib.h>

#include "msgQLib.h"
#include "pipeDrv.h"
#include "semLib.h"
#include "taskLib.h"
#include "vxWorks.h"

#include "headers/aktoren.h"
#include "headers/auftrag.h"
#include "headers/auftragsverwaltung.h"
#include "headers/busdata.h"
#include "headers/data_types.h"
#include "headers/ea_steuerung.h"
#include "headers/aktordaten_gen.h"
#include "headers/job_queue.h"
#include "headers/konsolen_io.h"
#include "headers/lagerregal.h"
#include "headers/positionsbestimmung.h"
#include "headers/priorities.h"
#include "headers/schlittensteuerung.h"
#include "headers/sensordata.h"

#define MS_TO_NS(t) 1000*1000

#define TIMER_INTERVAL_SEC  0
#define TIMER_INTERVAL_NSEC 100000000 // 100000000


void timer_handler();

extern auftrag_t active_task;

MSG_Q_ID msgq_auftraege, msgq_target_pos;

timer_t timer_id;
struct itimerspec timer_spec;

const vec3_t POS_EINGABE = {1, 1, Z_EA};
const vec3_t POS_AUSGABE = {10, 1, Z_EA};

sbusdata sensor_raw;

lagerzustand_t lagerstatus = { {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}, {0, 0, Z_TURM}, {LS_AUS, LS_AN, LS_AUS}, EA_VOLL, EA_LEER};

// beinhaltet die aktuellen Aktionen der Aktoren -> daraus müssten Busdaten generiert werden
aktor_status_t aktoren[5] = {STOP, STOP, STOP, STOP, STOP}; // TODO: Aktoren -> Busdaten
Aktorbits aktorbits;
//lichtschrankenstatus_t lichtschranken[3] = {LS_AUS, LS_AUS, LS_AUS};

TASK_ID task_id_av, task_id_sstrg, task_id_eastrg, task_id_adg, task_id_rdsd, task_id_posb;

SEM_ID sem_lagerstatus;

int start(void) {
	init_auftragsverwaltung();
	init_read_sensor_data();
	init_sende_aktordaten();
	
	sem_lagerstatus = semBCreate(SEM_Q_FIFO, SEM_FULL);
	
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
	timer_spec.it_interval.tv_sec = TIMER_INTERVAL_SEC;
	timer_spec.it_interval.tv_nsec = TIMER_INTERVAL_NSEC;
	if (timer_create(CLOCK_REALTIME, NULL, &timer_id) == ERROR)
	{
		printf("ERROR: Creating timer failed!");
		return(errno);
	}
	// timer_callback mit Timer verbinden
	if (timer_connect(timer_id, (VOIDFUNCPTR) timer_handler, NULL) == ERROR)
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
	
	printf("msgq_target_pos: %p\n", msgq_target_pos);
	
//	auftrag_t test = {AUSLAGERN, WARTET, {4, 2}};

	taskDelay(100);
	
//	if((msgQSend(msgq_auftraege, (char*)(void*)&test, MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
//	{
//		printf("ERROR: msgQSend for msgq_auftraege failed!\n");
//	}
	
	taskDelay(10);
	taskPrioritySet(taskIdSelf(), PRIO_LOW);
	
	while(1){
	}
}

int i = 0;
void timer_handler() {
	i++;
//	printf("\n--- %d ---\n", i);
//	if(i > 100000)
//	{
//		printf("END.");
//		taskDelete(taskIdSelf());
//	}
	
	if (active_task.status == FERTIG || active_task.status == STATUS_UNINITIALIZED)
		verarbeiteKommando();
	
	// Sensordaten von Simulation empfangen & auswerten
	task_id_rdsd = taskSpawn("rdsd", PRIO_HIGH, 0x100, 2000, (FUNCPTR) readSensorData, &sensor_raw, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	task_id_posb = taskSpawn("posb", PRIO_HIGH, 0x100, 2000, (FUNCPTR) positionBestimmen, &sensor_raw, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	// Aktoren ansteuern	
	task_id_av = taskSpawn("av", PRIO_HIGH, 0x100, 2000, (FUNCPTR) auftragsverwaltung, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	task_id_sstrg = taskSpawn("sstrg", PRIO_HIGH, 0x100, 2000, (FUNCPTR) schlittensteuerung, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	task_id_eastrg = taskSpawn("eastrg", PRIO_HIGH, 0x100, 2000, (FUNCPTR) ea_steuerung, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	// Aktordaten an Simulation schicken
	task_id_adg = taskSpawn("adg", PRIO_HIGH, 0x100, 2000, (FUNCPTR) taskSendeAktorDaten, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
