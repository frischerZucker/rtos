//sensordaten.c
#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <pipeDrv.h>
#include <ioLib.h>
#include <unistd.h>
#include <semLib.h>

#include "headers/busdata.h"
#include "headers/lagerhardware.h"  //?
#include "headers/pipes.h"
#include "headers/sensordaten_gen.h"

extern SEM_ID semLagerhardware;  
extern regal_status lager;       //Sensorwerte

//extern speicher_zustand zustand;

extern int sensor_pipe_fd;

// Initialisierung der Pipe
//void initSensorPipe(void) {
//    pipeDrv(10);
//    pipeDevCreate(SENSOR_PIPE_NAME, SENSOR_PIPE_MSG_SIZE, 10);
//
//    pipeFdSensor = open(SENSOR_PIPE_NAME, O_WRONLY, 0);
//    if (pipeFdSensor == ERROR) {
//        perror("Fehler beim Oeffnen der Pipe");
//    }
//}

// Sensorwerte generieren und senden
void sensordaten_gen(void) {
    sbusdata daten;
    memset(&daten, 0, sizeof(sbusdata));

    if (semTake(semLagerhardware, WAIT_FOREVER) == OK) {
		//Y-Tastsensoren unten
		daten.sbits.y1u = (lager.y < 5);
		daten.sbits.y2u = (lager.y >= 10 && lager.y < 15);
		daten.sbits.y3u = (lager.y >= 20 && lager.y < 25);
		daten.sbits.y4u = (lager.y >= 30 && lager.y < 35);
		daten.sbits.y5u = (lager.y >= 40 && lager.y < 45);
	
		//Y-Tastsensoren oben
		daten.sbits.y1o = (lager.y >= 5 && lager.y < 10);
		daten.sbits.y2o = (lager.y >= 15 && lager.y < 20);
		daten.sbits.y3o = (lager.y >= 25 && lager.y < 30);
		daten.sbits.y4o = (lager.y >= 35 && lager.y < 40);
		daten.sbits.y5o = (lager.y >= 45);
	
		//X-Tastsensoren
		daten.sbits.x1  = (lager.x < 10);
		daten.sbits.x2  = (lager.x >= 10 && lager.x < 20);
		daten.sbits.x3  = (lager.x >= 20 && lager.x < 30);
		daten.sbits.x4  = (lager.x >= 30 && lager.x < 40);
		daten.sbits.x5  = (lager.x >= 40 && lager.x < 50);
		daten.sbits.x6  = (lager.x >= 50 && lager.x < 60);
		daten.sbits.x7  = (lager.x >= 60 && lager.x < 70);
		daten.sbits.x8  = (lager.x >= 70 && lager.x < 80);
		daten.sbits.x9  = (lager.x >= 80 && lager.x < 90);
		daten.sbits.x10 = (lager.x >= 90);
	
		//Z-Tastsensoren
//		daten.sbits.zR  = (lager.z <= 5);   
//		daten.sbits.zM  = (lager.z == 10); // mitte
//		daten.sbits.zEA = (lager.z >= 15);  
		
//		zustand.z = 100;
//		zustand.y = 200;
//		zustand.x = 300;
		
		if (lager.z <= 5) {
			daten.sbits.zR = 1;
			daten.sbits.zM = 0;
			daten.sbits.zEA = 0;
		}
		else if (lager.z >= 15) {
			daten.sbits.zR = 0;
			daten.sbits.zM = 0;
			daten.sbits.zEA = 1;
		}
		else{
			daten.sbits.zR = 0;
			daten.sbits.zM = 1;
			daten.sbits.zEA = 0;
		}
		
		printf("ztest: lager.z: %d, zR: %d, zM: %d, zEA: %d\n ", lager.z, daten.sbits.zR, daten.sbits.zM, daten.sbits.zEA);
	
		//Lichtschranken
		daten.sbits.lL = 0;//(lager.x % lager.dist_x < lager.pin_x); // links
		daten.sbits.lR = 1;//(lager.x % lager.dist_x > lager.dist_x - lager.pin_x); // rechts
	
		//Lichttaster
		daten.sbits.lT = 1;  //?
	
		semGive(semLagerhardware);
    }

    // Senden �ber Pipe
    if (write(sensor_pipe_fd, &daten, sizeof(sbusdata)) <= 0) {
        perror("Fehler beim Schreiben in die Pipe");
    }
}
