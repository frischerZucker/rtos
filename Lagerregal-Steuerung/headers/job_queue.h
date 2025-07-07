/*
 * job_queue.h
 *
 *  Created on: Jul 3, 2025
 *      Author: moritz
 */
#ifndef HEADERS_JOB_QUEUE_H_
#define HEADERS_JOB_QUEUE_H_

#include "auftrag.h"
#include "data_types.h"

typedef enum {
	INIT,		// erster Job nach Initialisierung
	MOVE,		// Aktoren zu Zielposition bewegen
	EINGABE,	// Eingabeförderband
	AUSGABE,	// Ausgabeförderband
} jobtyp_t;

// Liste mit einzelnen Schritten eines Auftrags
typedef struct {
	auftragsstatus_t job_status[7];
	jobtyp_t job_types[7];
	vec3_t targets[7];
	int active_job;
	int length;
} job_queue_t;

#endif /* HEADERS_JOB_QUEUE_H_ */
