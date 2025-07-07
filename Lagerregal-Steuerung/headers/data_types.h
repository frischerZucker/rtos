/*
 * auftragsverwaltung.h
 *
 *  Created on: Jul 1, 2025
 *      Author: moritz
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#define MAX_AUFTRAEGE 10
#define MAX_AUFTRAGS_MSG_LENGTH 100
#define MAX_TARGET_POS 1

// falls definiert werden Infos zum Zustand des Systems ausgegeben
#define DEBUG

// 3x1 Vektor -> zB. für Position
typedef struct {
	int x;
	int y;
	int z;
} vec3_t;

// 2x1 Vektor -> zB. für Zielfach im Regal eines Auftrags
typedef struct {
	int x;
	int y;
} vec2_t;

#endif // DATA_TYPES_H
