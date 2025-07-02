/*
 * auftragsverwaltung.h
 *
 *  Created on: Jul 1, 2025
 *      Author: moritz
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

// 3x1 Vektor -> zB. für Position
typedef struct {
	double x;
	double y;
	double z;
} vec3_t;

// 2x1 Vektor -> zB. für Zielfach im Regal eines Auftrags
typedef struct {
	double x;
	double y;
} vec2_t;

typedef enum {
	Z_REGAL,
	Z_TURM,
	Z_EA
} pos_z_t;

extern const vec3_t POS_EINGABE;
extern const vec3_t POS_AUSGABE;

// beschreibt ob etwas ins Lager ein- oder ausgelagert werden soll
typedef enum {
	TYPE_UNINITIALIZED,
	EINlAGERN,
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

#endif // DATA_TYPES_H
