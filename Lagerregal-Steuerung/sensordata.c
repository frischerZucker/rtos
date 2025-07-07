#include <stdio.h>
#include <string.h>

#include "ioLib.h"

#include "headers/lagerregal.h"
#include "headers/pipes.h"
#include "headers/sensordata.h"

extern lagerzustand_t lagerstatus;

int sensor_pipe_fd;

// Initialisiert alle Sensorbits mit 0
void sdata_init(sbusdata* sdata) {
    memset(sdata, 0, sizeof(sbusdata));
}

void init_read_sensor_data() {
	sensor_pipe_fd = open(SENSOR_PIPE_NAME, O_RDONLY);
	if (sensor_pipe_fd == ERROR) {
		printf("[ERROR] Sensor-Pipe konnte nicht geöffnet werden\n");
		return;
	}
}

// Simuliert das Auslesen der Sensordaten
// Hier kann die Belegung beliebig angepasst werden
void readSensorData(sbusdata* data) {
	sdata_init(data);
	
	int bytes = 0;
	if (ioctl(sensor_pipe_fd, FIONREAD, &bytes) == ERROR) {
		printf("ERROR: ioctl failed!\n");
		return;
	}
	    	
	// abbrechen wenn keine Nachricht vorhanden ist
	if (bytes <= 0) {
		printf("[Sensordaten] keine Nachricht vorhanden\n");
	    return;
	}
	    	
	// Nachricht lesen und ausgeben
	char buffer[100];
	read(sensor_pipe_fd, buffer, sizeof(buffer));
	
	*data = *(sbusdata *) (void *) buffer;
	
	lagerstatus.lichtschranken[LS_EINGABE] = (data->sbits.lL) ? LS_AN : LS_AUS;
	lagerstatus.lichtschranken[LS_AUSGABE] = (data->sbits.lR) ? LS_AN : LS_AUS;
	lagerstatus.lichtschranken[LS_TURM] = (data->sbits.lT) ? LS_AN : LS_AUS;
	
	
}
