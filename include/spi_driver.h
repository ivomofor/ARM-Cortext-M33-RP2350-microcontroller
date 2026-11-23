#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

// Simulated SPI device context.
typedef struct
{
    bool selected;
    uint8_t registers[5];

} spi_device_t;

// Initialize the SPI communication driver.
bool spi_driver_initialize(void);

// Read a register from the SPI device.
bool spi_driver_read_register(uint8_t address, uint8_t *value);

// Write a value to a register on the SPI device.
bool spi_driver_write_register(uint8_t address, uint8_t value);

// Initialize a specific SPI device.
bool spi_device_initialize(spi_device_t *device);

// Read a register from a specific SPI device.
bool spi_device_read_register(spi_device_t *device,
                              uint8_t address,
                              uint8_t *value);

// Write a value to a register on a specific SPI device.
bool spi_device_write_register(spi_device_t *device,
                               uint8_t address,
                               uint8_t value);

#endif
