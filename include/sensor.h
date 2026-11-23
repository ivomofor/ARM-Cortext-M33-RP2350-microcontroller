#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#include "communication_interface.h"

// Sensor driver error codes.
typedef enum
{
    SENSOR_ERROR_NONE = 0,
    SENSOR_ERROR_INVALID_ARGUMENT,
    SENSOR_ERROR_COMMUNICATION_INITIALIZATION,
    SENSOR_ERROR_DEVICE_ID_READ,
    SENSOR_ERROR_UNEXPECTED_DEVICE_ID,
    SENSOR_ERROR_CONFIGURATION_WRITE,
    SENSOR_ERROR_CONFIGURATION_READ,
    SENSOR_ERROR_CONFIGURATION_VERIFICATION,
    SENSOR_ERROR_TEMPERATURE_MSB_READ,
    SENSOR_ERROR_TEMPERATURE_LSB_READ

} sensor_error_t;

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
sensor_error_t sensor_initialize(sensor_t *sensor);

// Read the sensor temperature.
sensor_error_t sensor_read_temperature(sensor_t *sensor,
                             uint16_t *temperature);

#endif
