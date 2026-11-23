#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "communication_interface_spi.h"

// Initialize the SPI communication interface.
static bool spi_interface_initialize(void *context)
{
    // Convert the generic context into an SPI device pointer.
    spi_device_t *device = (spi_device_t *)context;

    // Validate the device pointer.
    if (device == NULL)
    {
        return false;
    }

    // Ensure the device is not selected.
    device->selected = false;

    return true;
}

// Read a register through the SPI communication interface.
static bool spi_interface_read_register(void *context,
                                        uint8_t address,
                                        uint8_t *value)
{
    // Convert the generic context into an SPI device pointer.
    spi_device_t *device = (spi_device_t *)context;

    return spi_device_read_register(device, address, value);
}

// Write a register through the SPI communication interface.
static bool spi_interface_write_register(void *context,
                                         uint8_t address,
                                         uint8_t value)
{
    // Convert the generic context into an SPI device pointer.
    spi_device_t *device = (spi_device_t *)context;

    return spi_device_write_register(device, address, value);
}

// Configure an SPI communication interface for a specific device.
void spi_communication_interface_configure(
    communication_interface_t *interface,
    spi_device_t *device)
{
    // Validate the pointers.
    if (interface == NULL || device == NULL)
    {
        return;
    }

    // Configure the interface.
    interface->context = device;
    interface->initialize = spi_interface_initialize;
    interface->read_register = spi_interface_read_register;
    interface->write_register = spi_interface_write_register;
}