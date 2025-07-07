/*
 * auftragsverwaltung.h
 *
 *  Created on: Jul 1, 2025
 *      Author: moritz
 */

#ifndef AUFTRAGSVERWALTUNG_H
#define AUFTRAGSVERWALTUNG_H

#include "job_queue.h"

#define IS_LAST_JOB(jq) (jq.active_job >= jq.length - 1)

void init_auftragsverwaltung();
void auftragsverwaltung();
void auftragsverwaltung_loop();

job_queue_t generate_job_queue(auftrag_t auftrag);

#endif // AUFTRAGSVERWALTUNG_H
