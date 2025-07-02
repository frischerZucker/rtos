/*
 * auftragsverwaltung.c
 *
 *  Created on: Jul 1, 2025
 *      Author: moritz
 */

#include <stdio.h>

#include "msgQLib.h"
#include "semLib.h"

#include "auftragsverwaltung.h"
//#include "data_types.h"
#include "schlittensteuerung.h"

#define DEBUG

#define MAX_AUFTRAGS_MSG_LENGTH 512

// MsgQueue, über die Aufträge von I/O-Verwaltung empfangen werden
extern MSG_Q_ID msgq_auftraege;
// MsgQueue um Aktorsteuerung Zielposition zu übermitteln
extern MSG_Q_ID msgq_target_pos;

// Semaphore um Schlittensteurung zu blockieren (zB. wenn EA aktiv)
SEM_ID lock_schlitten;

// auszuführender Auftrag
auftrag_t active_task;
// Liste mit einzelnen Schritten zur Erfüllung des Auftrags
job_queue_t job_queue;

void init_auftragsverwaltung() {
	active_task.type = TYPE_UNINITIALIZED;
	active_task.status = STATUS_UNINITIALIZED;
	active_task.target.x = 0;
	active_task.target.y = 0;
	lock_schlitten = semBCreate(SEM_Q_FIFO, SEM_FULL);
}

void auftragsverwaltung() {
#ifdef DEBUG
	printf("TASK - Status: %d\nTyp: %d\nTarget: (%2f,%2f)\n", active_task.status, active_task.type, active_task.target.x, active_task.target.y);
	int j = job_queue.active_job;
	printf("JOB\nStatus: %d\nType:%d\nTarget: (%f,%f)\n", job_queue.job_status[j], job_queue.job_types[j], job_queue.targets[j].x, job_queue.targets[j].y);
#endif
	
	// überprüft ob ein neuer Auftrag bearbeitet werden kann
	if (active_task.status == FERTIG || active_task.status == STATUS_UNINITIALIZED) {
		// keine Aufträge verfügbar -> Task beenden
		if (msgQNumMsgs(msgq_auftraege) <= 0)
			return;

		// neuen Auftrag aus MsgQ auslesen
		char msg_buffer[MAX_AUFTRAGS_MSG_LENGTH];
		if (msgQReceive(msgq_auftraege, msg_buffer, MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER) == ERROR) {
			printf("msgQReceive in taskTwo failed\n");
			return;
		}
		// Pointermagie um Auftrag aus char-buffer zu entziffern
		active_task = *(auftrag_t*) (void*) msg_buffer;

		// Liste mit einzelnen abzuarbeitenden Schritten für Auftrag generieren 
		job_queue = generate_job_queue(active_task);
		
		active_task.status = BEARBEITUNG;
	}
	
	// falls Job beendet, nächsten Job starten
	if (job_queue.job_status[job_queue.active_job] == FERTIG) {
		printf("test %d\n\n\n", job_queue.active_job);
		// falls letzter Job -> Aufgabe erfüllt
		if (IS_LAST_JOB(job_queue)){
			// TODO: Belegungsdaten aktualisieren
			
			// Auftragsstatus auf FERTIG setzen, damit beim nächsten Aufruf ein neuer Auftrag geladen wird
			active_task.status = FERTIG;
			
			// TODO: Auftragsbestätigung "ERFÜLLT" senden
			
			return;
		}
		
		// nächsten Job auswählen
		job_queue.active_job += 1;
		printf("test2 %d\n\n\n", job_queue.active_job);
		switch (job_queue.job_types[job_queue.active_job]) {
			case MOVE:
				if((msgQSend(msgq_target_pos, (char*)(void*)&job_queue.targets[job_queue.active_job], MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
					printf("ERROR: sending new target position failed!\n");
				break;
			case EINGABE:
				semTake(lock_schlitten, WAIT_FOREVER);
				// TODO: Startsignal an Eingabe schicken
				break;
			case AUSGABE:
				semTake(lock_schlitten, WAIT_FOREVER);
				// TODO: Startsignal an Ausgabe schicken
				break;
			default:
				break;
		}
	}
}

job_queue_t generate_job_queue(auftrag_t auftrag) {
	job_queue_t jq;
	
	jq.active_job = 0;
	jq.length = 6;
	
	// INIT-Job FERTIG, Rest WARTET
	jq.job_status[0] = FERTIG;
	for (int i = 1; i < 7; i++) {
		jq.job_status[i] = WARTET;
	}
	jq.job_status[1] = FERTIG;
	
	jq.job_types[0] = INIT;
	if (auftrag.type == EINlAGERN) {
		jq.job_types[1] = INIT;
		jq.job_types[2] = MOVE; // zur Eingabe bewegen
		jq.targets[2].x = POS_EINGABE.x;
		jq.targets[2].y = POS_EINGABE.y;
		jq.targets[2].z = POS_EINGABE.z;
		jq.job_types[3] = EINGABE; // Eingabe
		jq.job_types[4] = MOVE; // zum Ziel y oben, z drinnen fahren
		jq.targets[4].x = auftrag.target.x;
		jq.targets[4].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[4].z = Z_REGAL;
		jq.job_types[5] = MOVE; // zum Ziel y unten fahren
		jq.targets[5].x = auftrag.target.x;
		jq.targets[5].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[5].z = Z_REGAL;
		jq.job_types[6] = MOVE; // zu z Mitte fahren
		jq.targets[6].x = auftrag.target.x;
		jq.targets[6].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[6].z = Z_TURM;
	}
	else { // AUSLAGERN
		jq.job_types[1] = INIT;
		jq.job_types[2] = MOVE; // zum Ziel y unten, z drinnen fahren
		jq.targets[2].x = auftrag.target.x;
		jq.targets[2].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[2].z = Z_REGAL;
		jq.job_types[3] = MOVE; // zum Ziel y oben fahren
		jq.targets[3].x = auftrag.target.x;
		jq.targets[3].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[3].z = Z_REGAL;
		jq.job_types[4] = MOVE; // zu z Mitte fahren
		jq.targets[4].x = auftrag.target.x;
		jq.targets[4].y = auftrag.target.y; // TODO: oben und unten unterscheiden
		jq.targets[4].z = Z_TURM;
		jq.job_types[5] = MOVE; // zu Ausgabe fahren
		jq.targets[5].x = POS_AUSGABE.x;
		jq.targets[5].y = POS_AUSGABE.y;
		jq.targets[5].z = POS_AUSGABE.z;
		jq.job_types[6] = AUSGABE; // Ausgabe
	}
	
	return jq;
}
