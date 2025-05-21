#include <stdio.h>
#include <vxWorks.h>
#include <sysLib.h>
#include <taskLib.h>
#include <tickLib.h>
#include <time.h>
#include <timexLib.h>

#define TPS 100

#define MS_TO_NS 1000*1000
#define S_TO_MS 1000
#define MS_TO_S .001

typedef struct calc_distance_args
{
	double *p_distance_m;
	double v_mps;
	long now_ms;
} calc_distance_args_t;

/*
 * Timer
 * 1. Beispielprogramm zum Laufen bekommen
 * 2. TIMER_START, TIMER_INTERVAL, sysClkRateSet ver�ndern
 * -> TIMER_START: Sekunden bis Timer das erste Mal klingelt
 * -> TIMER_INTERVAL: Sekunden bis zum n�chsten Klingeln
 * -> sysClkRateSet: wie viele Ticks pro Sekunde der Timer hat -> legt somit kleinst m�gliches Intervall fest?
 * 3. alle 50ms Weg berechnen (t und v als Parameter �bergeben), alle 1s Gesamtweg ausgeben
 */

void timer_callback_print(timer_t timerid, double *distance_m);
void timer_callback_calc(timer_t timerid, void *args);
long get_time_ms();
double calc_distance(long t_ms, double v_mps);

SEM_ID sem_calc_distance_args;

int main(void)
{
	double distance_m = 0;
	
	// 100 Ticks pro Sekunde
	sysClkRateSet(TPS);
	
	timer_t calc_distance_timer_id, print_timer_id;
	struct itimerspec calc_distance_timer_spec, print_timer_spec;
	
	// Argumente für Callback von calc_distance_timer
	calc_distance_args_t calc_distance_args;
	calc_distance_args.p_distance_m = &distance_m;
	calc_distance_args.v_mps = .1;
	calc_distance_args.now_ms = 0;
	
	sem_calc_distance_args = semMCreate(SEM_Q_FIFO);
	
	/*
	 * Timer zur Berechnung der Strecke erstellen
	 */
	// Timer startet nach 2s, dann alle 50ms
	calc_distance_timer_spec.it_value.tv_sec = 2;
	calc_distance_timer_spec.it_value.tv_nsec = 0;
	calc_distance_timer_spec.it_interval.tv_sec = 0;
	calc_distance_timer_spec.it_interval.tv_nsec = 50 * MS_TO_NS;
	if (timer_create(CLOCK_REALTIME, NULL, &calc_distance_timer_id) == ERROR)
	{
		printf("ERROR: Creating timer failed!");
		return(errno);
	}
	// timer_callback mit Timer verbinden
	if (timer_connect(calc_distance_timer_id, (VOIDFUNCPTR) timer_callback_calc, (void *)&calc_distance_args) == ERROR)
	{
		printf("ERROR: Connecting calc_distance to the timer failed!");
		return(errno);
	}
	// Timer starten
	if (timer_settime(calc_distance_timer_id, TIMER_RELTIME, &calc_distance_timer_spec, NULL) == ERROR)
	{
		printf("ERROR: timer_settime failed!");
		return(errno);
	}
	
	/*
	 * Timer zur Ausgabe erstellen
	 */
	// Timer startet nach 2s, dann alle 50ms
	print_timer_spec.it_value.tv_sec = 2;
	print_timer_spec.it_value.tv_nsec = 0;
	print_timer_spec.it_interval.tv_sec = 1;
	print_timer_spec.it_interval.tv_nsec = 0;
	if (timer_create(CLOCK_REALTIME, NULL, &print_timer_id) == ERROR)
	{
		printf("ERROR: Creating timer failed!");
		return(errno);
	}
	// timer_callback mit Timer verbinden
	if (timer_connect(print_timer_id, (VOIDFUNCPTR) timer_callback_print, &distance_m) == ERROR)
	{
		printf("ERROR: Connecting calc_distance to the timer failed!");
		return(errno);
	}
	// Timer starten
	if (timer_settime(print_timer_id, TIMER_RELTIME, &print_timer_spec, NULL) == ERROR)
	{
		printf("ERROR: timer_settime failed!");
		return(errno);
	}

	/*
	 * Endlosschleife
	 */
	while(1)
	{
		semMTake(sem_calc_distance_args, WAIT_FOREVER);
		
		calc_distance_args.now_ms = get_time_ms(); // aktuelle Zeit merken
		
		semMGive(sem_calc_distance_args);
	}
	
	// Timer wieder ausschalten & löschen -> wird theoretisch nie erreicht
	if (timer_cancel(calc_distance_timer_id) == ERROR)
	{
		printf("ERROR: Canceling timer failed!");
		return(errno);
	}
	if (timer_delete(calc_distance_timer_id) == ERROR)
	{
		printf("ERROR: Deleting timer failed!");
		return(errno);
	}
	if (timer_cancel(print_timer_id) == ERROR)
	{
		printf("ERROR: Canceling timer failed!");
		return(errno);
	}
	if (timer_delete(print_timer_id) == ERROR)
	{
		printf("ERROR: Deleting timer failed!");
		return(errno);
	}
	
	return OK;
}

/*
 * Callback des Ausgabe-Timers -> gibt jede Sekunde die zurückgelegte Strecke zurück
 */
void timer_callback_print(timer_t timerid, double *distance_m)
{
	printf("zurueckgelegte Strecke: %f\n", *distance_m);
}

/*
 * Callback für calc_distance_timer, wird alle 50ms aufgerufen.
 * Berechnet die seit dem letzten Aufruf zurückgelegte Strecke und addiert sie zur Gesamtstrecke.
 * 
 * Argumente:
 * 	void *args	-> eigentlich calc_distance_args_t *, muss umgecasted werden
 * 				-> beinhaltet Geschwindigkeit [m/s], aktuelle Zeit [ms] & Pointer auf Variable fürs Ergebnis
 */
void timer_callback_calc(timer_t timerid, void *args)
{
	semMTake(sem_calc_distance_args, WAIT_FOREVER);
	
	static long last_time_ms = -1; // Zeitpunkt als die Funktion das letzte Mal ausgeführt wurde
	long dt_ms = 0;
	
	calc_distance_args_t *temp = (calc_distance_args_t *)args;
	
	if (last_time_ms == -1) last_time_ms = temp->now_ms;
	
	// Zeit seit letztem Aufruf berechnen
	dt_ms = temp->now_ms - last_time_ms;
	// Strecke berechnen
	*temp->p_distance_m += temp->v_mps * dt_ms * MS_TO_S;

	last_time_ms = temp->now_ms;
	
	semMGive(sem_calc_distance_args);
}

/*
 * Berechnet die vergangene Zeit in ms.
 */
long get_time_ms()
{
	return (tickGet() / TPS) * S_TO_MS;
}
