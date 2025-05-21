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
void timer_callback_calc(timer_t timerid, double *return_to);
int get_time_ms();
double calc_distance(int t_ms, double v_mps);

int main(void)
{
	double distance_m = 0;
	
	// 100 Ticks pro Sekunde
	sysClkRateSet(TPS);
	
	timer_t calc_distance_timer_id, print_timer_id;
	struct itimerspec calc_distance_timer_spec, print_timer_spec;
	
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
	if (timer_connect(calc_distance_timer_id, (VOIDFUNCPTR) timer_callback_calc, &distance_m) == ERROR)
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

	while(1)
	{
		// Endlosschleife während der der Timer ausgeführt wird
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
	// Timer wieder ausschalten & löschen -> wird theoretisch nie erreicht
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
 * Callback des Streckenberechnungs-Timers; wird alle 50ms ausgeführt.
 */
void timer_callback_calc(timer_t timerid, double *distance_m)
{
	static int last_time_ms = -1;
	int now_ms = 0, dt_ms = 0;
	double v_mps = .1;
	
	if (last_time_ms == -1) last_time_ms = get_time_ms();
	
	// Zeit seit letztem Aufruf berechnen
	now_ms = get_time_ms();
	dt_ms = now_ms - last_time_ms;
	// Strecke berechnen
	*distance_m += calc_distance(dt_ms, v_mps);

	last_time_ms = now_ms;
}

/*
 * Berechnet die Geschwindigkeit in m/s.
 */
double calc_distance(int t_ms, double v_mps)
{
	double t_s = t_ms * MS_TO_S;

	return v_mps * t_s;
}

/*
 * Berechnet die vergangene Zeit in ms.
 */
int get_time_ms()
{
	return (tickGet() / TPS) * S_TO_MS;
}
