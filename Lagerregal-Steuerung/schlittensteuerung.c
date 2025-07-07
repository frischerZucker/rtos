/*
 * schlittensteuerung.c
 *
 *  Created on: Jul 2, 2025
 *      Author: moritz
 */

#include "headers/schlittensteuerung.h"

#include <msgQLib.h>
#include <semLib.h>
#include <stdio.h>
#include <taskLib.h>

#include "headers/aktoren.h"
#include "headers/auftrag.h"
#include "headers/busdata.h"
#include "headers/data_types.h"
#include "headers/job_queue.h"
#include "headers/lagerregal.h"
#include "headers/priorities.h"

#define DEBUG

extern TASK_ID task_id_sstrg;

// MsgQueue um Zielposition zu erhalten
extern MSG_Q_ID msgq_target_pos;

// blockiert Schlitten wenn EA aktiv
extern SEM_ID schlitten_blockierung;

extern lagerzustand_t lagerstatus;

extern job_queue_t job_queue;

vec3_t target_pos = {-1, -1, -1};

// beinhaltet die aktuellen Aktionen der Aktoren -> daraus müssten Busdaten generiert werden
//extern aktor_status_t aktoren[5];
extern Aktorbits aktorbits;

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
#ifdef DEBUG
		printf("[Schlittensteuerung] Neues Target erhalten: (%d, %d, %d)\n", target_pos.x, target_pos.y, target_pos.z);
#endif // DEBUG
	}
	
	// ungültige Positionen nicht anfahren
	if (target_pos.x < 0 || target_pos.y < 0 || target_pos.z < 0) return;
	
	// Schlitten zum Richtigen X-Wert bewegen
	if (lagerstatus.schlitten_pos.x != target_pos.x) {
		// falls Schlitten nicht im Turm erst in Turm fahren -> sonst geht was am Regal kaputt
		if (lagerstatus.schlitten_pos.z != Z_TURM) {
			aktorbits.azh = (lagerstatus.schlitten_pos.z > Z_TURM) ? AKTOR_AUS : AKTOR_AN;
			aktorbits.azv = (lagerstatus.schlitten_pos.z > Z_TURM) ? AKTOR_AN : AKTOR_AUS;
#ifdef DEBUG
			printf("[Schlittensteuerung] Schlitten nicht mittig, starte Aktor Z nach %s\n", (lagerstatus.schlitten_pos.x > target_pos.x) ? "vorne" : "hinten");
#endif // DEBUG
			return;
		}
		
		aktorbits.azh = AKTOR_AUS;
		aktorbits.azv = AKTOR_AUS;

		aktorbits.axl = (lagerstatus.schlitten_pos.x > target_pos.x) ? AKTOR_AN : AKTOR_AUS;
		aktorbits.axr = (lagerstatus.schlitten_pos.x > target_pos.x) ? AKTOR_AUS : AKTOR_AN;
#ifdef DEBUG
		printf("[Schlittensteuerung] Stoppe Aktor Z, starte Aktor X nach %s\n", (lagerstatus.schlitten_pos.x > target_pos.x) ? "links" : "rechts");
#endif // DEBUG
		
		return;
	}
	
	aktorbits.axl = AKTOR_AUS;
	aktorbits.axr = AKTOR_AUS;
#ifdef DEBUG
	printf("[Schlittensteuerung] X erreicht, stoppe Aktor X\n");
#endif // DEBUG
	
	// Schlitten zum Richtigen Y-Wert bewegen
	if (lagerstatus.schlitten_pos.y != target_pos.y) {
		// falls Schlitten nicht im Turm erst in Turm fahren -> sonst geht was am Regal kaputt
		if (lagerstatus.schlitten_pos.z != Z_TURM) {
			aktorbits.azh = (lagerstatus.schlitten_pos.z > Z_TURM) ? AKTOR_AUS : AKTOR_AN;
			aktorbits.azv = (lagerstatus.schlitten_pos.z > Z_TURM) ? AKTOR_AN : AKTOR_AUS;
#ifdef DEBUG
			printf("[Schlittensteuerung] Schlitten nicht mittig, starte Aktor Z nach %s\n", (lagerstatus.schlitten_pos.x > target_pos.x) ? "vorne" : "hinten");
#endif // DEBUG
			return;
		}
		
		aktorbits.azh = AKTOR_AUS;
		aktorbits.azv = AKTOR_AUS;
		
		aktorbits.ayo = (lagerstatus.schlitten_pos.y > target_pos.y) ? AKTOR_AUS : AKTOR_AN;
		aktorbits.ayu = (lagerstatus.schlitten_pos.y > target_pos.y) ? AKTOR_AN : AKTOR_AUS;
#ifdef DEBUG
		printf("[Schlittensteuerung] Stoppe Aktor Z, starte Aktor Y nach %s\n", (lagerstatus.schlitten_pos.y > target_pos.x) ? "unten" : "oben");
#endif // DEBUG

		return;
	}
	aktorbits.ayo = AKTOR_AUS;
	aktorbits.ayu = AKTOR_AUS;
#ifdef DEBUG
	printf("[Schlittensteuerung] Y erreicht, stoppe Aktor Y\n");
#endif // DEBUG
		
	// Schlitten zum Richtigen Z-Wert bewegen
	if (lagerstatus.schlitten_pos.z != target_pos.z) {
		aktorbits.azh = (lagerstatus.schlitten_pos.z > target_pos.z) ? AKTOR_AUS : AKTOR_AN;
		aktorbits.azv = (lagerstatus.schlitten_pos.z > target_pos.z) ? AKTOR_AN : AKTOR_AUS;
#ifdef DEBUG
		printf("[Schlittensteuerung] Starte Aktor Z nach %s\n", (lagerstatus.schlitten_pos.x > target_pos.x) ? "hinten" : "vorne");
#endif // DEBUG
	}
	else { // Ziel erreicht
		aktorbits.azh = AKTOR_AUS;
		aktorbits.azv = AKTOR_AUS;
#ifdef DEBUG
		printf("[Schlittensteuerung] Target erreicht, stoppe Aktor Z\n");
#endif // DEBUG
		
		// Job als erfüllt markieren -> Auftragsverwaltung kann neuen Job zuteilen
		job_queue.job_status[job_queue.active_job] = FERTIG;
	}
}
