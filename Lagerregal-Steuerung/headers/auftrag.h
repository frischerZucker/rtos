/*
 * auftrag.h
 *
 *  Created on: Jul 3, 2025
 *      Author: moritz
 */

#ifndef HEADERS_AUFTRAG_H_
#define HEADERS_AUFTRAG_H_

#include "data_types.h"

// beschreibt ob etwas ins Lager ein- oder ausgelagert werden soll
typedef enum {
	TYPE_UNINITIALIZED,
	EINLAGERN,
	AUSLAGERN
} auftragstyp_t;

// aktueller Status des Auftrags
typedef enum {
	STATUS_UNINITIALIZED,
	WARTET,
	BEARBEITUNG,
	FERTIG
} auftragsstatus_t;

// Auftrag
typedef struct {
	auftragstyp_t type;
	auftragsstatus_t status;
	vec2_t target;
} auftrag_t;

#endif /* HEADERS_AUFTRAG_H_ */
