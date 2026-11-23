#include "sensor.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize and configure the simulated sensor.
sensor_error_t sensor_initialize(sensor_t *sensor)
{
    // Validate the communication interface.
    if (sensor == NULL ||
        sensor->communication.initialize == NULL ||
        sensor->communication.read_register == NULL ||
        sensor->communication.write_register == NULL)
    {
        printf("Sensor: Invalid sensor instance or communication interface.\n");
        return SENSOR_ERROR_INVALID_ARGUMENT;
    }

    // Step 1: Initialize the communication interface.
    if (!sensor->communication.initialize(sensor->communication.context))
    {
        printf("Sensor: Communication interface initialization failed.\n");
        return SENSOR_ERROR_COMMUNICATION_INITIALIZATION;
    }

    uint8_t device_id = 0;

    // Step 2: Read the device ID.
    printf("\nSensor: Initializing device...\n");

    if (!sensor->communication.read_register(sensor->communication.context,
                                             0x00,
                                             &device_id))
    {
        printf("Sensor: Failed to read device ID.\n");
        return SENSOR_ERROR_DEVICE_ID_READ;
    }

    // Step 3: Verify the device ID.
    if (device_id != sensor->configuration.expected_device_id)
    {
        printf("Sensor: Unexpected device ID 0x%02X\n", device_id);
        return SENSOR_ERROR_UNEXPECTED_DEVICE_ID;
    }

    printf("Sensor: Device ID verified: 0x%02X\n", device_id);

    // Step 4: Write the required configuration.
    if (!sensor->communication.write_register(sensor->communication.context,
                                              sensor->configuration.configuration_register,
                                              sensor->configuration.configuration_value))
    {
        printf("Sensor: Failed to configure device.\n");
        return SENSOR_ERROR_CONFIGURATION_WRITE;
    }

    // Step 5: Read the configuration back.
    uint8_t readback_value = 0;

    if (!sensor->communication.read_register(sensor->communication.context,
                                             sensor->configuration.configuration_register,
                                             &readback_value))
    {
        printf("Sensor: Failed to read configuration.\n");
        return SENSOR_ERROR_CONFIGURATION_READ;
    }

    // Step 6: Verify the configuration.
    if (readback_value != sensor->configuration.configuration_value)
    {
        printf("Sensor: Configuration verification failed!\n");
        return SENSOR_ERROR_CONFIGURATION_VERIFICATION;
    }

    printf("Sensor: Configuration verified: 0x%02X\n", readback_value);
    printf("Sensor: Device initialization successful!\n");

    return SENSOR_ERROR_NONE;
}

// Read the sensor temperature.
sensor_error_t sensor_read_temperature(sensor_t *sensor,
                             uint16_t *temperature)
{
    // Validate the sensor instance and communication interface.
    if (sensor == NULL ||
        sensor->communication.read_register == NULL)
    {
        printf("Sensor: Invalid sensor instance or communication interface.\n");
        return SENSOR_ERROR_INVALID_ARGUMENT;
    }

    // Validate the output pointer.
    if (temperature == NULL)
    {
        printf("Sensor: Invalid temperature output pointer.\n");
        return SENSOR_ERROR_INVALID_ARGUMENT;
    }

    // Read the temperature MSB.
    uint8_t msb = 0;

    if (!sensor->communication.read_register(sensor->communication.context,
                                             0x02,
                                             &msb))
    {
        printf("Sensor: Failed to read temperature MSB.\n");
        return SENSOR_ERROR_TEMPERATURE_MSB_READ;
    }

    // Read the temperature LSB.
    uint8_t lsb = 0;

    if (!sensor->communication.read_register(sensor->communication.context,
                                             0x03,
                                             &lsb))
    {
        printf("Sensor: Failed to read temperature LSB.\n");
        return SENSOR_ERROR_TEMPERATURE_LSB_READ;
    }

    // Combine both bytes into a 16-bit measurement.
    *temperature = ((uint16_t)msb << 8) | lsb;

    printf("Sensor: Temperature raw value: %u (0x%04X)\n",
           (unsigned int)*temperature,
           (unsigned int)*temperature);

    return SENSOR_ERROR_NONE;
}
