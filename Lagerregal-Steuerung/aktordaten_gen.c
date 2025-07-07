//aktordaten_gen.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semLib.h>
#include <ioLib.h>

#include "pipeDrv.h"

#include "headers/aktoren.h"
#include "headers/busdata.h"
#include "headers/pipes.h"

//Globale Ressourcen
//extern SEM_ID semNeueAktorDaten;     // Semaphore
extern Aktorbits aktorbits;  // Globale Variable mit aktordaten
int aktor_pipe_fd;                   

void init_sende_aktordaten(){	
	aktor_pipe_fd = open(AKTOR_PIPE_NAME, O_WRONLY, 0);
	if (aktor_pipe_fd == ERROR) {
	    perror("Fehler beim Oeffnen der Pipe");
	}
}

// Aktordaten in Busdaten verpacken und �ber Pipe schicken
void taskSendeAktorDaten(void) {
    abusdata busdaten;

        // Auf neue Aktordaten warten
//	if (semTake(semNeueAktorDaten, WAIT_FOREVER) == OK) {    	
            // Daten �bernehmen
    memset(&busdaten, 0, sizeof(abusdata));
    busdaten.abits = aktorbits;
    busdaten.amsg[sizeof(Aktorbits)] = '\0'; // optionales Null-Terminator Byte
    
    // Schreiben in die Pipe
    int written = write(aktor_pipe_fd, &busdaten, sizeof(abusdata));
    if (written != sizeof(abusdata)) {
    	perror("[SendeAktorDaten] Fehler beim Schreiben in die Pipe");
    } else {
    	printf("[SendeAktorDaten] Aktordaten gesendet\n");
	}
//    }
}
