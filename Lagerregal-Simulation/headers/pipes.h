/*
 * pipes.h
 *
 *  Created on: Jul 6, 2025
 *      Author: moritz
 */

#ifndef PIPES_H
#define PIPES_H

#include "busdata.h"

#define AKTOR_PIPE_NAME "/pipe/aktor_pipe"
#define AKTOR_PIPE_MSG_SIZE sizeof(abusdata)

#define SENSOR_PIPE_NAME "/pipe/sensor_pipe"
#define SENSOR_PIPE_MSG_SIZE sizeof(sbusdata)

typedef struct {
    int x, y, z;
} speicher_zustand;

#endif // PIPES_H
