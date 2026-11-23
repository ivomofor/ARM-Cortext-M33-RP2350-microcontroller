#ifndef COMMUNICATION_INTERFACE_SPI_H
#define COMMUNICATION_INTERFACE_SPI_H

#include "communication_interface.h"
#include "spi_driver.h"

// Configure an SPI communication interface for a specific device.
void spi_communication_interface_configure(
    communication_interface_t *interface,
    spi_device_t *device);

#endif