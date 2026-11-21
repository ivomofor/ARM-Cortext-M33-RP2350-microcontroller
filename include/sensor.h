#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#include "communication_interface.h"

// Initialize and configure the sensor.
bool sensor_initialize(const communication_interface_t *communication);

// Read the sensor temperature.
bool sensor_read_temperature(const communication_interface_t *communication,
                             uint16_t *temperature);

#endif