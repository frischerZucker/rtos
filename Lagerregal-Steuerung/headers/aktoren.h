/*
 * aktoren.h
 *
 *  Created on: Jul 6, 2025
 *      Author: moritz
 */

#ifndef AKTOREN_H
#define AKTOREN_H

#define AKTOR_X 0
#define AKTOR_Y 1
#define AKTOR_Z 2

#define AKTOR_EINGABE 3
#define AKTOR_AUSGABE 4

#define AKTOR_AN 1
#define AKTOR_AUS 0

// gibt an in welche Richtung sich ein Aktor bewegt
typedef enum {
	STOP,
	VORWAERTS,
	RUECKWAERTS,
	RECHTS,
	LINKS,
	OBEN,
	UNTEN,
	AN,
	AUS
} aktor_status_t;

#endif // AKTOREN_H
