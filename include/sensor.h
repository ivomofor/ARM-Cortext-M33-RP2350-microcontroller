#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#include "communication_interface.h"

// Sensor configuration.
typedef struct
{
    uint8_t expected_device_id;
    uint8_t configuration_register;
    uint8_t configuration_value;

} sensor_config_t;

// Initialize and configure the sensor.
bool sensor_initialize(const communication_interface_t *communication,
                       const sensor_config_t *configuration);

// Read the sensor temperature.
bool sensor_read_temperature(const communication_interface_t *communication,
                             uint16_t *temperature);

#endif