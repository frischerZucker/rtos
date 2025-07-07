#ifndef SENSORDATA_H
#define SENSORDATA_H

#include "busdata.h"

void init_read_sensor_data();

// Liest Sensordaten (Simulation oder von Hardware)
void readSensorData(sbusdata* data);

#endif
