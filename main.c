#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/stdio_usb.h"

#include "spi_driver.h"
#include "sensor.h"
#include "spi_driver_test.h"
#include "communication_interface_spi.h"

#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

volatile bool alarm_fired = false;
volatile bool repeating_timer_fired = false;
volatile bool heartbeat_timer_fired = false;
volatile bool status_timer_fired = false;
volatile bool button_irq_fired = false;
volatile bool button_released_irq_fired = false;

void button_interrupt_handler(uint gpio, uint32_t events)
{
    if (gpio == 14)
    {
        if (events & GPIO_IRQ_EDGE_FALL)
        {
            button_irq_fired = true;
        }

        if (events & GPIO_IRQ_EDGE_RISE)
        {
            button_released_irq_fired = true;
        }
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;

    alarm_fired = true;

    return 0;
}

bool repeating_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    repeating_timer_fired = true;

    return true;
}

bool heartbeat_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    static uint32_t heartbeat_count = 0;

    heartbeat_count++;

    heartbeat_timer_fired = true;

    if (heartbeat_count >= 5)
    {
        return false;
    }

    return true;
}

bool status_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    status_timer_fired = true;

    return true;
}

void scan_i2c_bus(void)
{
    printf("\nScanning I2C bus...\n");

    uint8_t probe_byte = 0x00;
    bool device_found = false;

    for (uint8_t address = 0x08; address <= 0x77; address++)
    {
        int result = i2c_write_timeout_us(
            i2c0,
            address,
            &probe_byte,
            1,
            false,
            1000
        );

        if (result == 1)
        {
            printf("I2C device found at address: 0x%02X\n", address);
            device_found = true;
        }
    }

    if (!device_found)
    {
        printf("No I2C devices found.\n");
    }

    printf("I2C scan complete.\n\n");
}

static const uint8_t simulated_sensor_registers[] =
{
    0x42,  // Register 0x00: Device ID
    0x01,  // Register 0x01: Configuration
    0xFF,  // Register 0x02: Temperature MSB
    0xFB,  // Register 0x03: Temperature LSB
    0x00,  // Register 0x04: Status
    0x12,  // Register 0x05: Measurement byte 1
    0x34,  // Register 0x06: Measurement byte 2
    0x56   // Register 0x07: Measurement byte 3
};

uint8_t simulated_sensor_read_register(uint8_t register_address)
{
    if (register_address >= sizeof(simulated_sensor_registers))
    {
        printf("Invalid register address: 0x%02X\n", register_address);
        return 0;
    }

    return simulated_sensor_registers[register_address];
}

uint32_t simulated_sensor_read_24bit_measurement(void)
{
    uint8_t byte1 = simulated_sensor_read_register(0x05);
    uint8_t byte2 = simulated_sensor_read_register(0x06);
    uint8_t byte3 = simulated_sensor_read_register(0x07);

    uint32_t measurement =
        ((uint32_t)byte1 << 16) |
        ((uint32_t)byte2 << 8)  |
        byte3;

    return measurement;
}

bool simulated_sensor_data_ready(void)
{
    uint8_t status = simulated_sensor_read_register(0x04);

    return (status & 0x01) != 0;
}

bool simulated_sensor_read_measurement(uint32_t *measurement)
{
    uint32_t wait_time_ms = 0;

    while (!simulated_sensor_data_ready() && wait_time_ms < 5000)
    {
        printf("Sensor data is not ready. Waiting...\n");

        sleep_ms(1000);

        wait_time_ms += 1000;
    }

    if (!simulated_sensor_data_ready())
    {
        printf("Sensor timeout! No measurement received.\n");
        return false;
    }

    *measurement = simulated_sensor_read_24bit_measurement();

    return true;
}

uint16_t simulated_sensor_read_temperature(void)
{
    uint8_t msb = simulated_sensor_read_register(0x02);
    uint8_t lsb = simulated_sensor_read_register(0x03);

    uint16_t raw_temperature = ((uint16_t)msb << 8) | lsb;

    return raw_temperature;
}

int16_t simulated_sensor_read_signed_temperature(void)
{
    uint8_t msb = simulated_sensor_read_register(0x02);
    uint8_t lsb = simulated_sensor_read_register(0x03);

    uint16_t raw_value = ((uint16_t)msb << 8) | lsb;

    return (int16_t)raw_value;
}

float simulated_sensor_convert_temperature(int16_t raw_value)
{
    return (float)raw_value / 10.0f;
}

int main(void)
{
    stdio_init_all();

    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }

    uart_init(uart0, 9600);
    i2c_init(i2c0, 100000);
    uint baudrate = spi_init(spi0, 1000000);

    printf("SPI0 initialised. Requested baud rate: 1000000 Hz\n");
    printf("SPI0 actual baud rate: %u Hz\n", baudrate);

    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);

    gpio_pull_up(4);
    gpio_pull_up(5);

    gpio_set_function(16, GPIO_FUNC_SPI);
    gpio_set_function(18, GPIO_FUNC_SPI);
    gpio_set_function(19, GPIO_FUNC_SPI);

    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 1);

    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    uint8_t spi_tx_data = 0xA5;
    uint8_t spi_rx_data = 0x00;

    gpio_put(17, 0);

    spi_write_read_blocking(spi0, &spi_tx_data, &spi_rx_data, 1);

    gpio_put(17, 1);

    printf("SPI transmitted: 0x%02X\n", spi_tx_data);
    printf("SPI received:    0x%02X\n", spi_rx_data);

    if (spi_tx_data == spi_rx_data)
    {
        printf("SPI loopback test PASSED!\n");
    }
    else
    {
        printf("SPI loopback test FAILED!\n");
    }

    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);

    uart_puts(uart0, "Hello UART!\n");

    printf("UART transmission attempted.\n");
    stdio_flush();

    if (cyw43_arch_init())
    {
        printf("CYW43 initialization failed!\n");
        return 1;
    }

    printf("USB CDC is working!\n");
    stdio_flush();

    printf("Pico 2 W LED control started!\n");

    uint8_t device_id = simulated_sensor_read_register(0x00);

    printf("Simulated sensor device ID: 0x%02X\n", device_id);

    uint16_t raw_temperature = simulated_sensor_read_temperature();

    if (simulated_sensor_data_ready())
    {
        printf("New measurement available!\n");
    }
    else
    {
        printf("No new measurement available.\n");
    }

    printf("Simulated raw temperature: %u\n",
           (unsigned int)raw_temperature);

    printf("Simulated raw temperature (hex): 0x%04X\n",
           (unsigned int)raw_temperature);

    int16_t signed_temperature = simulated_sensor_read_signed_temperature();

    printf("Simulated signed temperature: %d C\n",
           (int)signed_temperature);

    float temperature_celsius = simulated_sensor_convert_temperature(signed_temperature);

    printf("Simulated temperature: %.1f C\n", (double)temperature_celsius);

    uint32_t measurement;

    if (simulated_sensor_read_measurement(&measurement))
    {
        printf("New measurement available!\n");

        printf("Simulated 24-bit measurement: 0x%06X\n",
               (unsigned int)measurement);
    }

    scan_i2c_bus();

    add_alarm_in_ms(2000, alarm_callback, NULL, false);

    repeating_timer_t repeating_timer;
    repeating_timer_t heartbeat_timer;
    repeating_timer_t status_timer;

    if (!add_repeating_timer_ms(2000, repeating_timer_callback, NULL, &repeating_timer))
    {
        printf("Failed to start repeating timer!\n");
        return 1;
    }

    if (!add_repeating_timer_ms(1000, heartbeat_timer_callback, NULL, &heartbeat_timer))
    {
        printf("Failed to start heartbeat timer!\n");
        return 1;
    }

    if (!add_repeating_timer_ms(3000, status_timer_callback, NULL, &status_timer))
    {
        printf("Failed to start status timer!\n");
        return 1;
    }

    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0);

    gpio_init(14);
    gpio_set_dir(14, GPIO_IN);
    gpio_pull_up(14);

    gpio_set_irq_enabled_with_callback(
        14,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &button_interrupt_handler
    );

    uint64_t timer_start_time = time_us_64();
    bool timer_cancelled = false;

    printf("\n--- Simulated SPI Register Test ---\n");

    // Read the device ID register.
    uint8_t spi_device_id;

    if (spi_driver_read_register(0x00, &spi_device_id))
    {
        printf("SPI Device ID: 0x%02X\n", spi_device_id);
    }

    // Update the configuration register.
    spi_driver_write_register(0x01, 0x05);

    // Read the configuration register again.
    uint8_t configuration;

    if (spi_driver_read_register(0x01, &configuration))
    {
        printf("Configuration: 0x%02X\n", configuration);
    }

    // Test reading an invalid register.
    uint8_t invalid_value;

    bool invalid_read_success =
        spi_driver_read_register(0x05, &invalid_value);

    // Verify that the invalid address was rejected.
    bool invalid_read_rejected = !invalid_read_success;

    // Verify that the device was deselected.
    bool device_deselected_after_invalid_read =
        !spi_driver_test_is_device_selected();

    // Check both conditions.
    if (invalid_read_rejected &&
        device_deselected_after_invalid_read)
    {
        printf("Invalid register read test PASSED\n");
    }
    else
    {
        printf("Invalid register read test FAILED\n");
    }

    // Test writing to an invalid register.
    spi_driver_write_register(0x05, 0xAA);

    // Test writing to the read-only Device ID register.
    spi_driver_write_register(0x00, 0x99);

    // Read the Device ID again to verify it hasn't changed.
    if (spi_driver_read_register(0x00, &spi_device_id))
    {
        printf("Device ID after write attempt: 0x%02X\n", spi_device_id);
    }

    printf("\n");

    printf("\n--- SPI Read Command Test ---\n");

    // Create a read command for register 0x00.
    uint8_t read_command = 0x80;

    // Variable to receive the register data.
    uint8_t command_result;

    // Execute the valid raw read command.
    bool read_success =
        spi_driver_test_read_command(read_command, &command_result);

    printf("Read command: 0x%02X\n", read_command);
    printf("Read status:  %s\n", read_success ? "SUCCESS" : "FAILED");

    if (read_success)
    {
        printf("Read result:  0x%02X\n", command_result);
    }

    // Test an invalid read command.
    uint8_t invalid_command = 0x01;

    // Execute the invalid raw read command.
    bool invalid_command_success =
        spi_driver_test_read_command(invalid_command, &command_result);

    // Verify that the invalid command was rejected.
    bool invalid_command_rejected = !invalid_command_success;

    // Verify that the device was deselected.
    bool device_deselected_after_invalid_command =
        !spi_driver_test_is_device_selected();

    // Check both conditions.
    if (invalid_command_rejected &&
        device_deselected_after_invalid_command)
    {
        printf("Invalid read command test PASSED\n");
    }
    else
    {
        printf("Invalid read command test FAILED\n");
    }

    printf("\n");

    printf("\n--- SPI Write Command Test ---\n");

    // Create a write command for register 0x01.
    uint8_t write_command = 0x01;

    // Write 0x0A into register 0x01.
    spi_driver_test_write_command(write_command, 0x0A);

    // Read the register to verify the update.
    if (spi_driver_read_register(0x01, &configuration))
    {
        printf("Configuration after write: 0x%02X\n", configuration);
    }

    // Test an invalid write command.
    uint8_t invalid_write_command = 0x80;

    bool invalid_write_success =
        spi_driver_test_write_command(invalid_write_command, 0x55);

    // Verify that the device was deselected.
    bool device_deselected_after_invalid_write =
        !spi_driver_test_is_device_selected();

    // Verify that the Configuration register hasn't changed.
    bool configuration_read_success =
        spi_driver_read_register(0x01, &configuration);

    if (!invalid_write_success &&
        device_deselected_after_invalid_write &&
        configuration_read_success &&
        configuration == 0x0A)
    {
        printf("Invalid write command test PASSED\n");
    }
    else
    {
        printf("Invalid write command test FAILED\n");
    }

    if (configuration_read_success)
    {
        printf("Configuration after invalid command: 0x%02X\n",
               configuration);
    }

    // Test writing to the read-only Device ID register.
    bool read_only_write_success =
        spi_driver_test_write_command(0x00, 0x99);

    // Verify that the device was deselected.
    bool device_deselected_after_read_only_write =
        !spi_driver_test_is_device_selected();

    // Verify that the Device ID hasn't changed.
    bool device_id_read_success =
        spi_driver_read_register(0x00, &spi_device_id);

    // Check all conditions.
    if (!read_only_write_success &&
        device_deselected_after_read_only_write &&
        device_id_read_success &&
        spi_device_id == 0x42)
    {
        printf("Read-only register protection test PASSED\n");
    }
    else
    {
        printf("Read-only register protection test FAILED\n");
    }

    if (device_id_read_success)
    {
        printf("Device ID after write command: 0x%02X\n",
               spi_device_id);
    }

    printf("\n");

    printf("\n--- SPI Chip Select Test ---\n");

    // Verify the initial state.
    printf("Initial selection state: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    // Select the SPI device.
    spi_driver_test_select_device();

    // Verify the selected state.
    printf("Selection state: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    // Deselect the SPI device.
    spi_driver_test_deselect_device();

    // Verify the deselected state.
    printf("Final selection state: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    printf("\n");

    printf("\n--- Complete SPI Read Transaction Test ---\n");

    // Variable to receive the register data.
    uint8_t transaction_result;

    // Perform the complete SPI read transaction.
    bool transaction_success =
        spi_driver_read_register(0x00, &transaction_result);

    // Display the transaction status.
    printf("Transaction status: %s\n",
           transaction_success ? "SUCCESS" : "FAILED");

    // Display the received data if successful.
    if (transaction_success)
    {
        printf("Transaction result: 0x%02X\n", transaction_result);
    }

    // Verify that the device was deselected.
    printf("Final selection state: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    printf("\n");

    printf("\n--- Complete SPI Write Transaction Test ---\n");

    // Prepare the write command and data.
    uint8_t transaction_write_command = 0x01;
    uint8_t transaction_write_data = 0x0A;

    // Perform the complete SPI write transaction.
    bool write_transaction_success =
        spi_driver_write_register(transaction_write_command,
                                  transaction_write_data);

    // Verify that the device was deselected after the write.
    bool device_deselected_after_write =
        !spi_driver_test_is_device_selected();

    // Read back the Configuration register.
    uint8_t written_value;

    bool read_back_success =
        spi_driver_read_register(0x01, &written_value);

    // Verify that the write transaction succeeded.
    if (write_transaction_success &&
        device_deselected_after_write &&
        read_back_success &&
        written_value == transaction_write_data)
    {
        printf("Complete SPI write transaction test PASSED\n");
    }
    else
    {
        printf("Complete SPI write transaction test FAILED\n");
    }

    // Display the read-back value.
    if (read_back_success)
    {
        printf("Configuration register: 0x%02X\n", written_value);
    }

    // Verify the final selection state.
    printf("Final selection state: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    printf("\n");

    printf("\n--- SPI Invalid Register Address Test ---\n");

    // Attempt to read an invalid register.
    uint8_t invalid_read_result;

    bool invalid_address_success =
        spi_driver_read_register(0x20, &invalid_read_result);

    // Verify that the device was deselected after the failed transaction.
    bool device_deselected_after_invalid_address =
        !spi_driver_test_is_device_selected();

    // Verify that the invalid address was rejected
    // and the device was deselected.
    if (!invalid_address_success &&
        device_deselected_after_invalid_address)
    {
        printf("Invalid register address test PASSED\n");
    }
    else
    {
        printf("Invalid register address test FAILED\n");
    }

    // Display the device selection state.
    printf("Device selection after invalid read: %s\n",
           device_deselected_after_invalid_address ? "NOT SELECTED" : "SELECTED");

    // Attempt to write to an invalid register.
    bool invalid_write_register_success =
        spi_driver_write_register(0x20, 0x55);

    // Verify that the device was deselected.
    bool device_deselected_after_invalid_register_write =
        !spi_driver_test_is_device_selected();

    // Check both conditions.
    if (!invalid_write_register_success &&
        device_deselected_after_invalid_register_write)
    {
        printf("Invalid register write test PASSED\n");
    }
    else
    {
        printf("Invalid register write test FAILED\n");
    }

    printf("\n--- SPI Transaction Failure Recovery Test ---\n");

    uint8_t recovery_test_value;

    // Attempt a transaction using an invalid register address.
    bool recovery_test_success =
        spi_driver_read_register(0x20, &recovery_test_value);

    // Verify that the failed transaction left the device deselected.
    bool device_deselected_after_failure =
        !spi_driver_test_is_device_selected();

    // Verify that the driver rejected the invalid transaction
    // and correctly deselected the SPI device.
    if (!recovery_test_success &&
        device_deselected_after_failure)
    {
        printf("SPI transaction failure recovery test PASSED\n");
    }
    else
    {
        printf("SPI transaction failure recovery test FAILED\n");
    }

    printf("Device selection after failure: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    printf("\n--- SPI Communication After Recovery Test ---\n");

    // Retry communication by reading the Device ID register.
    bool recovery_retry_success =
        spi_driver_read_register(0x00, &recovery_test_value);

    // Verify that the device was deselected after the transaction.
    bool device_deselected_after_retry =
        !spi_driver_test_is_device_selected();

    // Verify that communication recovered successfully.
    if (recovery_retry_success &&
        recovery_test_value == 0x42 &&
        device_deselected_after_retry)
    {
        printf("SPI communication recovery test PASSED\n");
    }
    else
    {
        printf("SPI communication recovery test FAILED\n");
    }

    if (recovery_retry_success)
    {
        printf("Recovered Device ID: 0x%02X\n",
               recovery_test_value);
    }

    printf("Device selection after retry: %s\n",
           spi_driver_test_is_device_selected() ? "SELECTED" : "NOT SELECTED");

    printf("\n--- SPI Timeout Tests ---\n");

    /* Test 1: Device responds before timeout */
    bool spi_ready_success = spi_driver_test_wait_until_ready(20000, true);

    if (spi_ready_success)
    {
        printf("SPI readiness test PASSED\n");
    }
    else
    {
        printf("SPI readiness test FAILED\n");
    }

    /* Test 2: Device does not respond */
    bool spi_ready_timeout = spi_driver_test_wait_until_ready(0, false);

    if (!spi_ready_timeout)
    {
        printf("SPI timeout test PASSED\n");
    }
    else
    {
        printf("SPI timeout test FAILED\n");
    }

    printf("\n");

    // Define the sensor configuration.
    sensor_config_t sensor_configuration =
    {
        .expected_device_id = 0x42,
        .configuration_register = 0x01,
        .configuration_value = 0x05
    };

    printf("\n--- NULL Communication Interface Test ---\n");

    bool null_interface_test =
        !sensor_initialize(NULL, &sensor_configuration);

    if (null_interface_test)
    {
        printf("NULL communication interface test PASSED\n");
    }
    else
    {
        printf("NULL communication interface test FAILED\n");
    }

    printf("\n");

    printf("\n--- NULL Sensor Configuration Test ---\n");

    bool null_configuration_test =
        !sensor_initialize(&spi_communication_interface, NULL);

    if (null_configuration_test)
    {
        printf("NULL sensor configuration test PASSED\n");
    }
    else
    {
        printf("NULL sensor configuration test FAILED\n");
    }

    printf("\n--- SPI Device Initialization Test ---\n");

    bool initialization_success =
        sensor_initialize(&spi_communication_interface, &sensor_configuration);

    if (initialization_success)
    {
        printf("SPI initialization test PASSED\n");
    }
    else
    {
        printf("SPI initialization test FAILED\n");
    }

    printf("\n");

    printf("\n--- SPI Device Initialization Failure Test ---\n");

    // Simulate an unexpected Device ID.
    spi_driver_test_set_device_id(0x99);

    // Initialization should fail.
    bool initialization_failure_test =
        !sensor_initialize(&spi_communication_interface, &sensor_configuration);

    // Restore the expected Device ID.
    spi_driver_test_set_device_id(0x42);

    // Read back the restored Device ID.
    uint8_t restored_device_id = 0;

    bool device_id_restored =
        spi_driver_read_register(0x00, &restored_device_id);

    if (initialization_failure_test &&
        device_id_restored &&
        restored_device_id == 0x42)
    {
        printf("SPI initialization failure test PASSED\n");
    }
    else
    {
        printf("SPI initialization failure test FAILED\n");
    }

    printf("Device ID restored: 0x%02X\n", restored_device_id);

    printf("\n");

    printf("\n--- SPI Multi-Byte Temperature Reading Test ---\n");

    uint16_t temperature = 0;

    bool temperature_read_success =
        sensor_read_temperature(&spi_communication_interface, &temperature);

    if (temperature_read_success)
    {
        printf("Temperature reading test PASSED\n");
        printf("Temperature: %u (0x%04X)\n",
               (unsigned int)temperature,
               (unsigned int)temperature);
    }
    else
    {
        printf("Temperature reading test FAILED\n");
    }

    printf("\n");

    printf("\n--- SPI Temperature NULL Pointer Test ---\n");

    bool null_pointer_test =
        !sensor_read_temperature(&spi_communication_interface, NULL);

    if (null_pointer_test)
    {
        printf("SPI NULL pointer test PASSED\n");
    }
    else
    {
        printf("SPI NULL pointer test FAILED\n");
    }

    printf("\n");

    printf("\n--- Complete SPI Sensor Driver Test ---\n");

    uint16_t final_temperature = 0;

    bool sensor_test_success = false;

    // Step 1: Initialize the sensor.
    if (sensor_initialize(&spi_communication_interface, &sensor_configuration))
    {
        // Step 2: Read the temperature measurement.
        if (sensor_read_temperature(&spi_communication_interface, &final_temperature))
        {
            sensor_test_success = true;
        }
    }

    // Step 3: Report the complete test result.
    if (sensor_test_success)
    {
        printf("Complete SPI sensor test PASSED\n");

        printf("Final temperature reading: %u\n",
               (unsigned int)final_temperature);
    }
    else
    {
        printf("Complete SPI sensor test FAILED\n");
    }

    printf("\n");

    while (true)
    {
        if (uart_is_readable(uart0))
        {
            char received_char = uart_getc(uart0);

            printf("%c", received_char);
        }

        if (alarm_fired)
        {
            alarm_fired = false;

            printf("Hardware timer alarm fired!\n");
        }

        if (repeating_timer_fired)
        {
            repeating_timer_fired = false;

            printf("Repeating timer fired!\n");
        }

        if (heartbeat_timer_fired)
        {
            heartbeat_timer_fired = false;

            printf("Heartbeat timer fired!\n");
        }

        if (status_timer_fired)
        {
            status_timer_fired = false;

            printf("Status timer fired!\n");
        }

        uint64_t current_time = time_us_64();

        if (!timer_cancelled && current_time - timer_start_time >= 10000000)
        {
            if (cancel_repeating_timer(&repeating_timer))
            {
                printf("Repeating timer cancelled!\n");
            }

            timer_cancelled = true;
        }

        if (button_irq_fired)
        {
            button_irq_fired = false;

            gpio_put(15, 1);

            printf("Button pressed - LED ON\n");
        }

        if (button_released_irq_fired)
        {
            button_released_irq_fired = false;

            gpio_put(15, 0);

            printf("Button released - LED OFF\n");
        }
    }

    cyw43_arch_deinit();

    return 0;
}
