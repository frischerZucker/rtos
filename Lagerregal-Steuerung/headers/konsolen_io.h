#ifndef KONSOLEN_IO_H
#define KONSOLEN_IO_H

#define AUFTRAGS_MSG_SIZE sizeof(auftrag_t)

BOOL istGueltigePosition(int x, int y);

void zeigeBelegung(void);
void verarbeiteKommando(void);

#endif // KONSOLEN_IO_H
