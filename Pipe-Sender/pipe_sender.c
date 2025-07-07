#include <vxWorks.h>
#include <pipeDrv.h>
#include <ioLib.h>
#include <stdio.h>
#include <strLib.h>
#include <taskLib.h>

void start_sender(void) {
    pipeDrv(10);  // Treiber laden (nur nötig, wenn noch nicht geschehen)

    // Pipe anlegen (nur einmal im Gesamtsystem!)
    pipeDevCreate("/pipe/test", 10, 100);  

    // Pipe öffnen
    int fd = open("/pipe/test", O_WRONLY, 0);
    if (fd == ERROR) {
        printf("Sender: Fehler beim Oeffnen der Pipe.\n");
        return;
    }

    for (int i = 0; i < 30; i++) {
    	taskDelay(100);
    	
    	// nicht jedes Mal Nachricht senden -> Empfänger erhält so nicht immer eine Nachricht
    	if (i % 3 != 0) continue;
    	
    	const char* msg = "Hallo vom Projekt 1";
    	
    	// Nachricht in Pipe senden
    	write(fd, msg, strlen(msg) + 1);
    	printf("Sender: Nachricht gesendet.\n");
    }
    	
    close(fd);
    pipeDevDelete("/pipe/test", TRUE);
    
    printf("Sender: ENDE.");
}
