#include "communication_interface.h"

#include "spi_driver.h"

const communication_interface_t spi_communication_interface =
{
    .initialize = spi_driver_initialize,
    .read_register = spi_driver_read_register,
    .write_register = spi_driver_write_register
};