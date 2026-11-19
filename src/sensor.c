#include "sensor.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "spi_driver.h"

// Initialize and configure the simulated sensor.
bool sensor_initialize(void)
{
    // Step 1: Initialize the SPI communication driver.
    if (!spi_driver_initialize())
    {
        printf("Sensor: SPI driver initialization failed.\n");
        return false;
    }

    uint8_t device_id = 0;

    // Step 2: Read the device ID.
    printf("\nSensor: Initializing device...\n");

    if (!spi_driver_read_register(0x00, &device_id))
    {
        printf("Sensor: Failed to read device ID.\n");
        return false;
    }

    // Step 3: Verify the device ID.
    if (device_id != 0x42)
    {
        printf("Sensor: Unexpected device ID 0x%02X\n", device_id);
        return false;
    }

    printf("Sensor: Device ID verified: 0x%02X\n", device_id);

    // Step 4: Write the required configuration.
    if (!spi_driver_write_register(0x01, 0x05))
    {
        printf("Sensor: Failed to configure device.\n");
        return false;
    }

    // Step 5: Read the configuration back.
    uint8_t configuration = 0;

    if (!spi_driver_read_register(0x01, &configuration))
    {
        printf("Sensor: Failed to read configuration.\n");
        return false;
    }

    // Step 6: Verify the configuration.
    if (configuration != 0x05)
    {
        printf("Sensor: Configuration verification failed!\n");
        return false;
    }

    printf("Sensor: Configuration verified: 0x%02X\n", configuration);
    printf("Sensor: Device initialization successful!\n");

    return true;
}

// Read the sensor temperature.
bool sensor_read_temperature(uint16_t *temperature)
{
    // Validate the output pointer.
    if (temperature == NULL)
    {
        printf("Sensor: Invalid temperature output pointer.\n");
        return false;
    }

    // Read the temperature MSB.
    uint8_t msb = 0;

    if (!spi_driver_read_register(0x02, &msb))
    {
        printf("Sensor: Failed to read temperature MSB.\n");
        return false;
    }

    // Read the temperature LSB.
    uint8_t lsb = 0;

    if (!spi_driver_read_register(0x03, &lsb))
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
