#include <stdio.h>

#include "semLib.h"

#include "headers/lagerregal.h"
#include "headers/positionsbestimmung.h"

// Merkt sich die letzte gültige Y-Position (wird bei Sensor-Ausfall weiterverwendet)
static int y_last = 1; // Anfangswert kann angepasst werden

extern SEM_ID sem_lagerstatus;

extern lagerzustand_t lagerstatus;  // globale Lagerdatenstruktur

void positionBestimmen(const sbusdata* sdata) {
	semTake(sem_lagerstatus, WAIT_FOREVER);
	
//	lagerstatus.schlitten_pos.x = 0;
    int found = 0;
//    lagerstatus.schlitten_pos.z = sdata->sgroups.sZ;
    if (sdata->sbits.zEA) lagerstatus.schlitten_pos.z = 2;
    if (sdata->sbits.zM) lagerstatus.schlitten_pos.z = 1;
    if (sdata->sbits.zR) lagerstatus.schlitten_pos.z = 0;
    
    // Bestimme X-Position
    for (int i = 0; i < 10; ++i) {
        if ((sdata->sgroups.sX >> i) & 1) {
        	lagerstatus.schlitten_pos.x = i + 1;
            break;
        }
    }

    // Prüfe zuerst auf Y-oben
    for (int i = 0; i < 5; ++i) {
        if ((sdata->sgroups.sYo >> i) & 1) {
        	lagerstatus.schlitten_pos.y = (i + 1) * 2;
            y_last = lagerstatus.schlitten_pos.y;
            found = 1;
            break;
        }
    }
    // Wenn kein Y-oben gefunden, prüfe Y-unten
    if (!found) {
        for (int i = 0; i < 5; ++i) {
            if ((sdata->sgroups.sYu >> i) & 1) {
            	lagerstatus.schlitten_pos.y = (i + 1) * 2 - 1;
                y_last = lagerstatus.schlitten_pos.y;
                found = 1;
                break;
            }
        }
    }
    // Wenn weder Y-oben noch Y-unten gefunden: nutze die letzte gültige Y-Position
    if (!found) {
        lagerstatus.schlitten_pos.y = y_last;
    }

    semGive(sem_lagerstatus);
    
    printf("[Positionsbestimmung] X=%d, Y=%d, Z=%d\n", lagerstatus.schlitten_pos.x, lagerstatus.schlitten_pos.y, lagerstatus.schlitten_pos.z);
}
