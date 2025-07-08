#include <stdio.h>
#include <string.h>
#include <msgQLib.h>
#include <stdbool.h>

#include "headers/auftrag.h"
#include "headers/lagerregal.h"
#include "headers/konsolen_io.h"
#include "headers/readcommand.h"

extern lagerzustand_t lagerstatus;  // globale Lagerdatenstruktur
extern MSG_Q_ID msgq_auftraege;

// Prüft ob Position gültig im Regal ist
BOOL istGueltigePosition(int x, int y) {
    return (x > 0 && x <= COLS && y > 0 && y <= ROWS);
}

// zeigt die Belegung des Regals an
void zeigeBelegung(void) {
    printf("---------------------\n");
    for (int y = ROWS; y >= 1; y--) {  // reihen
        printf("|");
        for (int x = 1; x <= COLS; x++) {
            printf("%d|", lagerstatus.regal[x][y]);  //spalten
        }
        printf("\n");
    }
    printf("---------------------\n");
}

/*
 * Führt das eingegebene Kommando aus.
 * 
 * Ich hab keine Ahnung warum, aber manchmal kommt die Kommandoabfrage erneut, obwohl ein Kommando erfolgreich eingelesen wurde.
 * -> wenn nach Eingabe eines Befehls eine Kommandoeingabe ohne Fehlermeldung kommt, einfach nochmal Enter drücken
 */
void verarbeiteKommando(void) {
    char cmd_buffer[32];
    command cmd = readcommand(cmd_buffer);

    if(cmd.no_input) return;
    
    if (!cmd.parse_ok) {
        printf("Ungueltiges Kommando!\n");
        return;
    }
    //kommandostructur: kommando spalte zeile
    int x = cmd.par1;//spalte
    int y = cmd.par2;//zeile

    if (strcmp(cmd.cmd, "getspace") == 0) {
        zeigeBelegung();
    }
    else if (strcmp(cmd.cmd, "vsetspace") == 0) {
        if (istGueltigePosition(x, y)) {
        	lagerstatus.regal[x][y] = VOLL;
            printf("Platz [%d][%d] als belegt markiert.\n", x, y);
        } else {
            printf("Ungueltige Position.\n");
        }
    }
    else if (strcmp(cmd.cmd, "clearspace") == 0) {
        if (istGueltigePosition(x, y)) {
        	lagerstatus.regal[x][y] = LEER;
            printf("Platz [%d][%d] freigegeben.\n", x, y);
        } else {
            printf("Ungueltige Position.\n");
        }
    }
    else if (strcmp(cmd.cmd, "insert") == 0) {
        if (!istGueltigePosition(x, y)) {
            printf("Ungueltige Position.\n");
            return;
        }
        if (lagerstatus.regal[x][y] == VOLL) {
            printf("Position [%d][%d] ist bereits belegt.\n", x, y);
            return;
        }
        if (lagerstatus.eingabe == EA_LEER) {
            printf("Kein Kloetzchen am Eingabe-Slot.\n");
            return;
        }
        // Auftrag erzeugen
        auftrag_t auftrag = {0};
        auftrag.type = EINLAGERN;
        auftrag.status = WARTET;
        auftrag.target.x = x;
        auftrag.target.y = y;
        //Auftrag schicken
        if (msgQSend(msgq_auftraege, (char*)&auftrag, AUFTRAGS_MSG_SIZE, NO_WAIT, MSG_PRI_NORMAL) == ERROR) {
            printf("Fehler beim Senden des Einlager-Auftrags");
            return;
        }
    }
    else if (strcmp(cmd.cmd, "remove") == 0) {
        if (!istGueltigePosition(x, y)) {
            printf("Ungueltige Position.\n");
            return;
        }
        if (lagerstatus.regal[x][y] == LEER) {
            printf("An Position [%d][%d] ist kein Kloetzchen.\n", x, y);
            return;
        }
        if (lagerstatus.ausgabe == EA_VOLL) {
            printf("Ausgabeplatz ist noch belegt.\n");
            return;
        }
        // Auftrag erzeugen
        auftrag_t auftrag = {0};
        auftrag.type = AUSLAGERN;
        auftrag.status = WARTET;
        auftrag.target.x = x;
        auftrag.target.y = y;

        if (msgQSend(msgq_auftraege, (char*)&auftrag, AUFTRAGS_MSG_SIZE, NO_WAIT, MSG_PRI_NORMAL) == ERROR) {
             printf("Fehler beim Senden des Auslager-Auftrags");
            return;
        }
        
    }
    else {
        printf("Unbekanntes Kommando: %s\n", cmd.cmd);
    }
}
