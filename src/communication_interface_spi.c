#include <stddef.h>

#include "communication_interface.h"
#include "spi_driver.h"

// Initialize the SPI communication interface.
static bool spi_interface_initialize(void *context)
{
    (void)context;

    return spi_driver_initialize();
}

// Read a register through the SPI communication interface.
static bool spi_interface_read_register(void *context,
                                        uint8_t address,
                                        uint8_t *value)
{
    (void)context;

    return spi_driver_read_register(address, value);
}

// Write a register through the SPI communication interface.
static bool spi_interface_write_register(void *context,
                                         uint8_t address,
                                         uint8_t value)
{
    (void)context;

    return spi_driver_write_register(address, value);
}

// SPI communication interface implementation.
const communication_interface_t spi_communication_interface =
{
    .context = NULL,
    .initialize = spi_interface_initialize,
    .read_register = spi_interface_read_register,
    .write_register = spi_interface_write_register
};