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
#include "data_types.h"

#define DEBUG

#define MAX_AUFTRAGS_MSG_LENGTH 512

// MsgQueue, über die Aufträge von I/O-Verwaltung empfangen werden
extern MSG_Q_ID msgq_auftraege;
// MsgQueue um Aktorsteuerung Zielposition zu übermitteln
extern MSG_Q_ID msgq_target_pos;

SEM_ID lock_schlitten;

// auszuführender Auftrag
auftrag_t active_task = { TYPE_UNINITIALIZED, STATUS_UNINITIALIZED, { 0, 0 } };
job_queue_t job_queue;

void init_auftragsverwaltung() {
	lock_schlitten = semBCreate(SEM_Q_FIFO, SEM_FULL);
}

void auftragsverwaltung() {
	printf("Status: %d\nTyp: %d\nTarget: (%2f,%2f)\n", active_task.status, active_task.type, active_task.target.x, active_task.target.y);
	
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

// TODO: generate_job_queue

job_queue_t generate_job_queue(auftrag_t auftrag) {
	
}
