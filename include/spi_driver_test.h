#ifndef SPI_DRIVER_TEST_H
#define SPI_DRIVER_TEST_H

#include <stdbool.h>
#include <stdint.h>


// Set the simulated device ID for testing.
void spi_driver_test_set_device_id(uint8_t device_id);
// Execute a raw SPI read command for testing.
bool spi_driver_test_read_command(uint8_t command, uint8_t *value);
// Execute a raw SPI write command for testing.
bool spi_driver_test_write_command(uint8_t command, uint8_t data);
// Check whether the simulated SPI device is selected.
bool spi_driver_test_is_device_selected(void);
// Select the simulated SPI device for testing.
void spi_driver_test_select_device(void);

// Deselect the simulated SPI device for testing.
void spi_driver_test_deselect_device(void);
// Wait for the simulated SPI device during testing.
bool spi_driver_test_wait_until_ready(uint64_t ready_delay_us,
                                      bool device_responds);

#endif
