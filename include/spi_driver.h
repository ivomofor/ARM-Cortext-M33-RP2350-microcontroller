#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

// Initialize the SPI communication driver.
bool spi_driver_initialize(void);

// Read a register from the SPI device.
bool spi_driver_read_register(uint8_t address, uint8_t *value);

// Write a value to a register on the SPI device.
bool spi_driver_write_register(uint8_t address, uint8_t value);

#endif
