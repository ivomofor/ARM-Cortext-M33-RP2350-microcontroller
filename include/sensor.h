#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

// Initialize and configure the sensor.
bool sensor_initialize(void);

// Read the sensor temperature.
bool sensor_read_temperature(uint16_t *temperature);

#endif
