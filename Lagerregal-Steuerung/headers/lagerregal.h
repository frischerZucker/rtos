/*
 * lagerregal.h
 *
 *  Created on: Jul 3, 2025
 *      Author: moritz
 */

#ifndef LAGERREGAL_H
#define LAGERREGAL_H

#include "data_types.h"

// Dimensionen des Lagers
#define ROWS 5
#define COLS 10

#define Y_OBEN(y) (y*2)
#define Y_UNTEN(y) (y*2-1)

// Positionen der Ein- & Ausgabe
extern const vec3_t POS_EINGABE;
extern const vec3_t POS_AUSGABE;

// mögliche Koordinaten der Z-Achse
typedef enum {
	Z_REGAL,
	Z_TURM,
	Z_EA
} pos_z_t;

// Zustand eines Fachs des Regals
typedef enum {
	LEER,
	VOLL
} regalzustand_t;

typedef enum {
	EA_LEER,
	EA_VOLL
} ea_zustand_t; 

// Indizes der Lichtschranken Lichtschranken im dazugehörigen Array
#define LS_EINGABE 0
#define LS_AUSGABE 1
#define LS_TURM 2

typedef enum {
	LS_AUS,
	LS_AN
} lichtschrankenstatus_t;

// Zustand des Lagers -> Fächer voll / leer, Position des Schlittens
typedef struct {
	regalzustand_t regal[COLS][ROWS];
	vec3_t schlitten_pos;
	lichtschrankenstatus_t lichtschranken[3];
	ea_zustand_t eingabe;
	ea_zustand_t ausgabe;
} lagerzustand_t;

#endif // LAGERREGAL_H
