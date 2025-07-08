#include <vxWorks.h>
#include <taskLib.h>
#include <semLib.h>
#include <pipeDrv.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ioLib.h>

#include "headers/busdata.h"
#include "headers/lagerhardware.h"
#include "headers/pipes.h"
#include "headers/sensordaten_gen.h"

#define TIMER_INTERVAL_SEC  0
#define TIMER_INTERVAL_NSEC 100000000 /*100000000*/

/* === Globale Ressourcen === */
int aktor_pipe_fd, sensor_pipe_fd;
MSG_Q_ID msgQ_gueltigeDaten;
SEM_ID semZustand;
SEM_ID semLagerhardware;
SEM_ID semStrecke;
timer_t timerStrecke;

/* === Globale Speicher === */
typedef struct {
    int zielX, zielY, zielZ;
} speicher_strecke;

speicher_zustand zustand;
speicher_strecke strecke;
regal_status lager;

/* ===== Busdaten analysieren (via Pipe) ===== */
void taskBusdatenAnalysieren(timer_t tid, int arg) {
    abusdata daten;
    
    int bytes = 0;
	if (ioctl(aktor_pipe_fd, FIONREAD, &bytes) == ERROR) {
        printf("ERROR: ioctl failed in taskBusdatenAnalysieren!\n");
    	return;
    }
        	
    // abbrechen wenn keine Nachricht vorhanden ist
	if (bytes <= 0) return;
    
    if (read(aktor_pipe_fd, &daten, sizeof(abusdata)) > 0) {
    	printf("[Simulation] Aktordaten erhalten\n");
    	printf("azv: %d azh: %d\n", daten.abits.azv, daten.abits.azh);
        /* === Bewegung simulieren in cm === */
        if (daten.abits.axr)
            lager.x += (daten.abits.axs ? lager.x_speed_fast : lager.x_speed);
        else if (daten.abits.axl)
            lager.x -= (daten.abits.axs ? lager.x_speed_fast : lager.x_speed);

        if (daten.abits.ayo)
            lager.y += lager.y_speed;
        else if (daten.abits.ayu)
            lager.y -= lager.y_speed;

        if (daten.abits.azv)
            lager.z -= lager.z_speed;
        else if (daten.abits.azh)
            lager.z += lager.z_speed;

        /* === Zustand bestimmen nur wenn Position im Toleranzbereich zur Lichtschranke === */
        int posX = (lager.x % lager.dist_x < lager.pin_x) ? (lager.x / lager.dist_x) : posX;
        int posY = (lager.y % lager.dist_yu < lager.pin_y) ? (lager.y / lager.dist_yu) : posY;
        int posZ = (lager.z % lager.dist_z < lager.pin_z) ? (lager.z / lager.dist_z) : posZ;
        
        sensordaten_gen();
        
        if (posX >= 0 && posX <= 9 && posY >= 0 && posY <= 4 && posZ >= 0 && posZ <= 2) {
            if (semTake(semZustand, WAIT_FOREVER) == OK) {
                zustand.x = posX;
                zustand.y = posY;
                zustand.z = posZ;
                semGive(semZustand);
            }

            msgQSend(msgQ_gueltigeDaten, (char*)&daten, sizeof(abusdata), NO_WAIT, MSG_PRI_NORMAL);

            // Streckenberechnung einmalig triggern
            struct itimerspec spec = { {0, 0}, {1, 0} };
            timer_settime(timerStrecke, TIMER_RELTIME, &spec, NULL);
        } else {
            printf("[Analyse] Position außerhalb Pin-Toleranz: x=%ld y=%ld z=%ld\n", lager.x, lager.y, lager.z);
        }
    }
    else {
    	printf("[Simulation] keine Daten erhalten\n\n");
    }
}

/* ===== Streckenberechnung ===== */
void taskStreckenberechnung(timer_t tid, int arg) {
    abusdata daten;

    if (msgQReceive(msgQ_gueltigeDaten, (char*)&daten, sizeof(abusdata), NO_WAIT) != ERROR) {
        int x, y, z, dx, dy, dz;

        if (semTake(semZustand, WAIT_FOREVER) == OK) {
            x = zustand.x; y = zustand.y; z = zustand.z;
            semGive(semZustand);
        }

        if (semTake(semLagerhardware, WAIT_FOREVER) == OK) {
            dx = lager.dist_x; dy = lager.dist_yu; dz = lager.dist_z;
            semGive(semLagerhardware);
        }

        if (semTake(semStrecke, WAIT_FOREVER) == OK) {
            strecke.zielX = (x + dx <= 9) ? x + dx : x;
            strecke.zielY = (y + dy <= 4) ? y + dy : y;
            strecke.zielZ = (z + dz <= 2) ? z + dz : z;
            semGive(semStrecke);
        }

        printf("[Strecke] Neue Zielkoordinaten: (%d, %d, %d)\n", strecke.zielX, strecke.zielY, strecke.zielZ);
    }
}

/* ===== Initialisierung ===== */
void initSimulationTasks() {
    struct itimerspec timerSpec;
    timer_t timerAnalyse;

    pipeDrv(10);
    pipeDevCreate(AKTOR_PIPE_NAME, AKTOR_PIPE_MSG_SIZE, 10);
    aktor_pipe_fd = open(AKTOR_PIPE_NAME, O_RDONLY, 0);
    if (aktor_pipe_fd == ERROR) {
        printf("[ERROR] Aktor-Pipe konnte nicht geöffnet werden\n");
        return;
    }

    pipeDevCreate(SENSOR_PIPE_NAME, SENSOR_PIPE_MSG_SIZE, 10);
    int sensor_pipe_fd_read = open(SENSOR_PIPE_NAME, O_RDONLY, 0);
    if (sensor_pipe_fd_read == ERROR) {
        printf("[ERROR] Sensor-Pipe (read) konnte nicht geöffnet werden\n");
        return;
    }
    
    sensor_pipe_fd = open(SENSOR_PIPE_NAME, O_WRONLY, 0);
    if (sensor_pipe_fd == ERROR) {
        printf("[ERROR] Sensor-Pipe (write) konnte nicht geöffnet werden\n");
        return;
    }
    
    msgQ_gueltigeDaten  = msgQCreate(10, sizeof(abusdata), MSG_Q_PRIORITY);
    semZustand          = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
    semLagerhardware    = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
    semStrecke          = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

    // Initialisierung
    lager.x = lager.y = 0; lager.z = 10;
    lager.dist_x = 10; lager.dist_yu = 10; lager.dist_z = 10;
    lager.pin_x = 5; lager.pin_y = 5; lager.pin_z = 5;
    lager.x_speed = 1; lager.x_speed_fast = 2;
    lager.y_speed = 1; lager.z_speed = 1;
    
    zustand.x = 0; zustand.y = 0; zustand.z = 1;

    timerSpec.it_value.tv_sec = 1;
    timerSpec.it_value.tv_nsec = 0;
    timerSpec.it_interval.tv_sec = TIMER_INTERVAL_SEC;
    timerSpec.it_interval.tv_nsec = TIMER_INTERVAL_NSEC;

    if (timer_create(CLOCK_REALTIME, NULL, &timerAnalyse) == ERROR) {
    	printf("ERROR: timer_create\n");
    	return;
    }
    if (timer_connect(timerAnalyse, (VOIDFUNCPTR)taskBusdatenAnalysieren, 0) == ERROR){
    	printf("ERROR: timer_connect\n");
    	return;
    }
    if(timer_settime(timerAnalyse, TIMER_RELTIME, &timerSpec, NULL) == ERROR){
    	printf("ERROR: timer_settime\n");
    	return;
    }

    timer_create(CLOCK_REALTIME, NULL, &timerStrecke);
    timer_connect(timerStrecke, (VOIDFUNCPTR)taskStreckenberechnung, 0);
    
    printf("[Simulation] Simulation fertig initialisiert.\n");
    
    while(1);
}
