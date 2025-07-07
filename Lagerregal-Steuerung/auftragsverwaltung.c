/*
 * auftragsverwaltung.c
 *
 *  Created on: Jul 1, 2025
 *      Author: moritz
 */

#include "headers/auftragsverwaltung.h"

#include <msgQLib.h>
#include <semLib.h>
#include <stdio.h>
#include <taskLib.h>

#include "headers/auftrag.h"
#include "headers/data_types.h"
#include "headers/ea_steuerung.h"
#include "headers/job_queue.h"
#include "headers/lagerregal.h"
#include "headers/priorities.h"
#include "headers/schlittensteuerung.h"

extern SEM_ID sem_lagerstatus;

// MsgQueue, über die Aufträge von I/O-Verwaltung empfangen werden
extern MSG_Q_ID msgq_auftraege;
// MsgQueue um Aktorsteuerung Zielposition zu übermitteln
extern MSG_Q_ID msgq_target_pos;

// Zustand des Lagers -> wo ist was eingelagert, Position des Schlittens
extern lagerzustand_t lagerstatus;

// Semaphore um Schlittensteurung zu blockieren (zB. wenn EA aktiv)
SEM_ID schlitten_blockierung;

// auszuführender Auftrag
auftrag_t active_task;
// Liste mit einzelnen Schritten zur Erfüllung des Auftrags
job_queue_t job_queue;

void init_auftragsverwaltung() {
	active_task.type = TYPE_UNINITIALIZED;
	active_task.status = STATUS_UNINITIALIZED;
	active_task.target.x = -1;
	active_task.target.y = -1;
	schlitten_blockierung = semBCreate(SEM_Q_FIFO, SEM_FULL);
}

void auftragsverwaltung() {
#ifdef DEBUG
	printf("[Auftragsverwaltung] TASK - Status: %d Typ: %d Target: (%d,%d)\n",
			active_task.status, active_task.type, active_task.target.x,
			active_task.target.y);
	int j = job_queue.active_job;
	printf("[Auftragsverwaltung] JOB (%d) - Status: %d Type:%d Target: (%d,%d,%d)\n",
			job_queue.active_job,
			job_queue.job_status[j], job_queue.job_types[j],
			job_queue.targets[j].x, job_queue.targets[j].y, job_queue.targets[j].z);
#endif

	// überprüft ob ein neuer Auftrag bearbeitet werden kann
	if (active_task.status == FERTIG || active_task.status == STATUS_UNINITIALIZED) {
		// keine Aufträge verfügbar -> Task beenden
		if (msgQNumMsgs(msgq_auftraege) <= 0) {
			return;
		}

		// neuen Auftrag aus MsgQ auslesen
		char msg_buffer[MAX_AUFTRAGS_MSG_LENGTH];
		if (msgQReceive(msgq_auftraege, msg_buffer, MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER) == ERROR) {
			printf("ERROR: msgQReceive msgq_auftraege failed!\n");
			return;
		}
		// Pointermagie um Auftrag aus char-buffer zu entziffern
		active_task = *(auftrag_t*) (void*) msg_buffer;
#ifdef DEBUG
		printf("[Auftragsverwaltung] Neuen Auftrag geladen Typ=%d, Target=(%d, %d)\n", active_task.type, active_task.target.x, active_task.target.y);
#endif // DEBUG
		
		// Liste mit einzelnen abzuarbeitenden Schritten für Auftrag generieren 
		job_queue = generate_job_queue(active_task);

		active_task.status = BEARBEITUNG;
	}

	// falls Job beendet, nächsten Job starten
	if (job_queue.job_status[job_queue.active_job] == FERTIG) {
		// falls letzter Job -> Aufgabe erfüllt
		if (IS_LAST_JOB(job_queue)) {
			// Belegungsdaten aktualisieren
			semTake(sem_lagerstatus, WAIT_FOREVER);
			lagerstatus.regal[active_task.target.x][active_task.target.y] = (active_task.type == EINLAGERN) ? VOLL : LEER;
			semGive(sem_lagerstatus);
			
			// Auftragsstatus auf FERTIG setzen, damit beim nächsten Aufruf ein neuer Auftrag geladen wird
			active_task.status = FERTIG;

			// TODO: Auftragsbestätigung "ERFÜLLT" senden
			printf("[Auftragsverwaltung] Auftrag erfuellt.\n");
			
			return;
		}

		// nächsten Job auswählen
		job_queue.active_job += 1;

		switch (job_queue.job_types[job_queue.active_job]) {
		case MOVE:
#ifdef DEBUG
			printf("[Auftragsverwaltung] Job erledigt, sende neues Target: (%d, %d, %d)\n", job_queue.targets[job_queue.active_job].x, job_queue.targets[job_queue.active_job].y, job_queue.targets[job_queue.active_job].z);
#endif // DEBUG
			if ((msgQSend(msgq_target_pos, (char*) (void*) &job_queue.targets[job_queue.active_job], MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
				printf("ERROR: sending new target position failed!\n");
			break;
		case EINGABE:
			semTake(schlitten_blockierung, WAIT_FOREVER);
#ifdef DEBUG
			printf("[Auftragsverwaltung] Starte Eingabe\n");
#endif // DEBUG
			ea_start(EINGABE); // Eingabe starten
			break;
		case AUSGABE:
			semTake(schlitten_blockierung, WAIT_FOREVER);
#ifdef DEBUG
			printf("[Auftragsverwaltung] Starte Ausgabe\n");
#endif // DEBUG
			ea_start(AUSGABE); // Ausgabe starten
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

	jq.job_types[0] = INIT;
	if (auftrag.type == EINLAGERN) {
		jq.job_types[1] = MOVE; // zur Eingabe bewegen
		jq.targets[1].x = POS_EINGABE.x;
		jq.targets[1].y = Y_UNTEN(POS_EINGABE.y);
		jq.targets[1].z = POS_EINGABE.z;
		jq.job_types[2] = EINGABE; // Eingabe
		jq.job_types[3] = MOVE; // zum Ziel y oben, z drinnen fahren
		jq.targets[3].x = auftrag.target.x;
		jq.targets[3].y = Y_OBEN(auftrag.target.y);
		jq.targets[3].z = Z_REGAL;
		jq.job_types[4] = MOVE; // zum Ziel y unten fahren
		jq.targets[4].x = auftrag.target.x;
		jq.targets[4].y = Y_UNTEN(auftrag.target.y);
		jq.targets[4].z = Z_REGAL;
		jq.job_types[5] = MOVE; // zu z Mitte fahren
		jq.targets[5].x = auftrag.target.x;
		jq.targets[5].y = Y_UNTEN(auftrag.target.y);
		jq.targets[5].z = Z_TURM;
	} else { // AUSLAGERN
		jq.job_types[1] = MOVE; // zum Ziel y unten, z drinnen fahren
		jq.targets[1].x = auftrag.target.x;
		jq.targets[1].y = Y_UNTEN(auftrag.target.y);
		jq.targets[1].z = Z_REGAL;
		jq.job_types[2] = MOVE; // zum Ziel y oben fahren
		jq.targets[2].x = auftrag.target.x;
		jq.targets[2].y = Y_OBEN(auftrag.target.y);
		jq.targets[2].z = Z_REGAL;
		jq.job_types[3] = MOVE; // zu Ausgabe fahren
		jq.targets[3].x = POS_AUSGABE.x;
		jq.targets[3].y = Y_UNTEN(POS_AUSGABE.y);
		jq.targets[3].z = POS_AUSGABE.z;
		jq.job_types[4] = AUSGABE; // Ausgabe
		jq.job_types[5] = MOVE; // zu z Mitte fahren
		jq.targets[5].x = auftrag.target.x;
		jq.targets[5].y = Y_UNTEN(auftrag.target.y);
		jq.targets[5].z = Z_TURM;
	}

	return jq;
}
