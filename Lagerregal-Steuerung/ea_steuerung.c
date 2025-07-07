/*
 * ea_steuerung.c
 *
 *  Created on: Jul 2, 2025
 *      Author: moritz
 */

#include <semLib.h>
#include <stdbool.h>

#include "headers/aktoren.h"
#include "headers/busdata.h"
#include "headers/ea_steuerung.h"
#include "headers/lagerregal.h"

extern SEM_ID schlitten_blockierung;

// beinhaltet die aktuellen Aktionen der Aktoren -> daraus müssten Busdaten generiert werden
//extern aktor_status_t aktoren[5];
lichtschrankenstatus_t lichtschranken[3];

extern Aktorbits aktorbits;

BOOL ea_aktiv = false;

jobtyp_t job;

void ea_start(jobtyp_t j) {
	job = j;
	
	ea_aktiv = true;
	
	printf("[EA-Steuerung] Starte EA\n");
}

void ea_steuerung(){
	if(!ea_aktiv) return; // abbrechen, wenn EA nicht aktiv sein soll
	
	if(job == EINGABE) {
		// Paket liegt noch in Eingabe -> Aktor der Eingabe starten
		if (lichtschranken[LS_EINGABE] == AN && lichtschranken[LS_TURM] == AUS) {
			aktorbits.aealre = AKTOR_AN;
		}
		// Paket ist im Schlitten angekommen -> Aktor ausschalten & Schlitten wieder freigeben
		else {
			aktorbits.aealre = AKTOR_AUS;
			ea_aktiv = false;
			semGive(schlitten_blockierung);
		}
	}
	else { // AUSGABE
		// Paket liegt noch im Schlitten -> Aktor der Ausgabe starten
		if (lichtschranken[LS_AUSGABE] == AUS && lichtschranken[LS_TURM] == AN) {
			aktorbits.aealra = AKTOR_AN;
		}
		// Paket ist in Ausgabe angekommen -> Aktor ausschalten & Schlitten wieder freigeben
		else {
			aktorbits.aealra = AKTOR_AUS;
			ea_aktiv = false;
			semGive(schlitten_blockierung);
		}
	}
}
