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

// Sensor instance.
typedef struct
{
    communication_interface_t communication;
    sensor_config_t configuration;

} sensor_t;

// Initialize and configure the sensor.
bool sensor_initialize(sensor_t *sensor);

// Read the sensor temperature.
bool sensor_read_temperature(sensor_t *sensor,
                             uint16_t *temperature);

#endif