#include "sensor.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize and configure the simulated sensor.
bool sensor_initialize(const communication_interface_t *communication,
                       const sensor_config_t *configuration)
{
    // Validate the communication interface.
    if (communication == NULL ||
        configuration == NULL ||
        communication->initialize == NULL ||
        communication->read_register == NULL ||
        communication->write_register == NULL)
    {
        printf("Sensor: Invalid communication interface or configuration.\n");
        return false;
    }

    // Step 1: Initialize the communication interface.
    if (!communication->initialize(communication->context))
    {
        printf("Sensor: Communication interface initialization failed.\n");
        return false;
    }

    uint8_t device_id = 0;

    // Step 2: Read the device ID.
    printf("\nSensor: Initializing device...\n");

    if (!communication->read_register(communication->context,
                                      0x00,
                                      &device_id))
    {
        printf("Sensor: Failed to read device ID.\n");
        return false;
    }

    // Step 3: Verify the device ID.
    if (device_id != configuration->expected_device_id)
    {
        printf("Sensor: Unexpected device ID 0x%02X\n", device_id);
        return false;
    }

    printf("Sensor: Device ID verified: 0x%02X\n", device_id);

    // Step 4: Write the required configuration.
    if (!communication->write_register(communication->context,
                                       configuration->configuration_register,
                                       configuration->configuration_value))
    {
        printf("Sensor: Failed to configure device.\n");
        return false;
    }

    // Step 5: Read the configuration back.
    uint8_t readback_value = 0;

    if (!communication->read_register(communication->context,
                                      configuration->configuration_register,
                                      &readback_value))
    {
        printf("Sensor: Failed to read configuration.\n");
        return false;
    }

    // Step 6: Verify the configuration.
    if (readback_value != configuration->configuration_value)
    {
        printf("Sensor: Configuration verification failed!\n");
        return false;
    }

    printf("Sensor: Configuration verified: 0x%02X\n", readback_value);
    printf("Sensor: Device initialization successful!\n");

    return true;
}

// Read the sensor temperature.
bool sensor_read_temperature(const communication_interface_t *communication,
                             uint16_t *temperature)
{
    // Validate the communication interface.
    if (communication == NULL ||
        communication->read_register == NULL)
    {
        printf("Sensor: Invalid communication interface.\n");
        return false;
    }

    // Validate the output pointer.
    if (temperature == NULL)
    {
        printf("Sensor: Invalid temperature output pointer.\n");
        return false;
    }

    // Read the temperature MSB.
    uint8_t msb = 0;

    if (!communication->read_register(communication->context,
                                      0x02,
                                      &msb))
    {
        printf("Sensor: Failed to read temperature MSB.\n");
        return false;
    }

    // Read the temperature LSB.
    uint8_t lsb = 0;

    if (!communication->read_register(communication->context,
                                      0x03,
                                      &lsb))
    {
        printf("Sensor: Failed to read temperature LSB.\n");
        return false;
    }

    // Combine both bytes into a 16-bit measurement.
    *temperature = ((uint16_t)msb << 8) | lsb;

    printf("Sensor: Temperature raw value: %u (0x%04X)\n",
           (unsigned int)*temperature,
           (unsigned int)*temperature);

    return true;
}