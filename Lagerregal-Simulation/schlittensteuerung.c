/*
 * schlittensteuerung.c
 *
 *  Created on: Jul 2, 2025
 *      Author: moritz
 */

#include <msgQLib.h>
#include <semLib.h>
#include <stdio.h>

#include "data_types.h"
#include "schlittensteuerung.h"

#define MAX_AUFTRAGS_MSG_LENGTH 512

// MsgQueue um Zielposition zu erhalten
extern MSG_Q_ID msgq_target_pos;

//
extern SEM_ID schlitten_blockierung;

extern lagerstatus_t lagerstatus;

extern job_queue_t job_queue;

vec3_t target_pos = {0, 0, 0};

void schlittensteuerung() {
	// blockiert den Schlitten, wenn EA aktiv
	semTake(schlitten_blockierung, WAIT_FOREVER);
	semGive(schlitten_blockierung);
	
	// falls vorhanden neue Zielposition übernehmen
	if (msgQNumMsgs(msgq_target_pos) > 0) {
		// Zielposition aus MsgQ auslesen
		char msg_buffer[MAX_AUFTRAGS_MSG_LENGTH];
		if (msgQReceive(msgq_target_pos, msg_buffer, MAX_AUFTRAGS_MSG_LENGTH, WAIT_FOREVER) == ERROR) {
			printf("ERROR: msgQReceive from msgq_target_pos failed!\n");
			return;
		}
		// Pointermagie um Auftrag aus char-buffer zu entziffern
		target_pos = *(vec3_t*) (void*) msg_buffer;
	}
	
	// Schlitten zum Richtigen X-Wert bewegen
	if (lagerstatus.schlitten_pos.x != target_pos.x) {
		// falls Schlitten nicht im Turm erst in Turm fahren -> sonst geht was am Regal kaputt
		if (lagerstatus.schlitten_pos.z != Z_TURM) {
			// TODO: starte Aktor Z Richtung Mitte
			return;
		}
		
		// TODO: stoppe Aktor Z
		// TODO: starte Aktor X Richtung Ziel
		
		return;
	}
	// TODO: stoppe Aktor X
	
	// Schlitten zum Richtigen Y-Wert bewegen
	if (lagerstatus.schlitten_pos.y != target_pos.y) {
		// falls Schlitten nicht im Turm erst in Turm fahren -> sonst geht was am Regal kaputt
		if (lagerstatus.schlitten_pos.z != Z_TURM) {
			// TODO: starte Aktor Z Richtung Mitte
			return;
		}
		
		// TODO: stoppe Aktor Z
		// TODO: starte Aktor Y Richtung Ziel
		
		return;
	}
	// TODO: stoppe Aktor Y
		
	// Schlitten zum Richtigen Z-Wert bewegen
	if (lagerstatus.schlitten_pos.z != target_pos.z) {} // TODO: starte Aktor Z Richtung Ziel
	else {
		// TODO: stoppe Aktor Z
		
		// Job als erfüllt markieren -> Auftragsverwaltung kann neuen Job zuteilen
		job_queue.job_status[job_queue.active_job] = FERTIG;
	}
}
