# Firmware in C on ARM Cortext-M33 (RP2350) microcontroller

## Overview

This project is part of my embedded C and firmware development learning journey, **Building Embedded Systems from Scratch**.

The goal is to develop practical microcontroller firmware using the Raspberry Pi Pico 2 W, progressing from fundamental embedded C programming to peripheral communication, reusable device drivers, hardware abstraction, debugging, and testing.

The project uses modular C code, the Raspberry Pi Pico SDK, CMake, and Git.

## Hardware

- **Development board:** Raspberry Pi Pico 2 W
- **Microcontroller:** RP2350
- **Processor:** ARM Cortex-M33
- **Development computer:** Ubuntu Linux

## Development Environment

The project uses:

- Raspberry Pi Pico SDK 2.3.1
- ARM GNU Toolchain 13.2.1
- GCC cross compiler
- CMake
- GNU Make
- Git
- Python serial tools

The Pico SDK is located at:

```text
~/project/pico-sdk
```

The `PICO_SDK_PATH` environment variable points to this directory.

## Project Structure

```text
pico2-firmware/
── CMakeLists.txt
── README.md
── main.c
── include/
   ── communication_interface.h
   ── communication_interface_spi.h
   ── sensor.h
   ── spi_driver.h
   ── spi_driver_test.h
── src/
   ── communication_interface_spi.c
   ── sensor.c
   ── spi_driver.c
── build/
```

The `include/` directory contains public interfaces and driver declarations.

The `src/` directory contains the implementations of the communication adapter, sensor driver, and SPI driver.

The `build/` directory contains generated build files and firmware artifacts. It is excluded from Git.

## Building the Firmware

Configure the project:

```bash
cmake -S . -B build
```

Compile the firmware:

```bash
cmake --build build -j4
```

The build generates several firmware artifacts, including:

```text
build/pico2_firmware.elf
build/pico2_firmware.uf2
build/pico2_firmware.bin
build/pico2_firmware.hex
```

The UF2 file is used to flash the Raspberry Pi Pico 2 W.

## Flashing the Firmware

1. Disconnect the Pico 2 W from USB.
2. Hold down the BOOTSEL button.
3. While holding BOOTSEL, connect the board to the computer.
4. Release BOOTSEL.
5. Confirm that the RP2350 bootloader appears as a USB storage device.
6. Copy the firmware:

```bash
cp build/pico2_firmware.uf2 /media/$USER/RP2350/
sync
```

The Pico automatically restarts after flashing.

## Viewing USB Serial Output

The firmware uses USB CDC serial communication.

Start the serial monitor:

```bash
python3 -m serial.tools.miniterm /dev/ttyACM0 115200
```

Alternatively, use:

```bash
screen /dev/ttyACM0 115200
```

The firmware reports information about peripheral tests, communication, sensor initialization, temperature readings, and timer events.

To exit the Python serial monitor, press:

```text
Ctrl + ]
```

Exiting the serial monitor does not stop the firmware running on the Pico.

## Current Firmware Features

### Microcontroller Fundamentals

- Raspberry Pi Pico 2 W initialization.
- USB CDC serial communication.
- Onboard LED control using the CYW43 interface.
- GPIO programming.
- Hardware timer usage.
- Repeating timers and interrupt handling.

### Communication Peripherals

- UART communication.
- I2C initialization and bus scanning.
- SPI peripheral initialization.
- SPI loopback testing.
- SPI register read and write operations.
- SPI command validation.
- SPI chip-select management.
- SPI transaction timeout handling.
- Transaction failure recovery.

### SPI Driver

The SPI driver provides a simulated SPI device implementation for developing and testing register based communication.

Features include:

- Register based device access.
- Device selection and deselection.
- Read and write command processing.
- Register address validation.
- Read only register protection.
- Multiple independent simulated SPI devices.
- Transaction status reporting.
- Timeout and failure recovery tests.

### Sensor Driver

The sensor driver provides a reusable interface for communicating with simulated sensor devices.

Features include:

- Sensor initialization.
- Device ID verification.
- Sensor configuration management.
- Configuration read back verification.
- Multi byte temperature readings.
- Multiple independent sensor instances.
- Structured error reporting.
- Argument validation.
- Communication error propagation.

## Firmware Architecture

The sensor and communication components are separated into independent modules.

The architecture follows this structure:

```text
             Application
               main.c
                 |
            Sensor Driver
              sensor.c
                 |
       Communication Interface
    communication_interface_spi.c
                 |
             SPI Driver
             spi_driver.c
                 |
       Simulated SPI Devices
```

### Hardware Abstraction

The sensor driver does not directly depend on the SPI implementation.

Instead, it communicates through a generic communication interface containing function pointers for initialization, register reading, and register writing.

The SPI adapter implements this interface and delegates communication to the SPI driver.

This separation provides:

- Modular firmware architecture.
- Reusable sensor drivers.
- Separation of application and hardware communication logic.
- Support for multiple independent device instances.
- Easier testing and future extension to other communication protocols.

## Testing and Integration

The project includes tests for SPI communication, sensor initialization, error handling, and multiple independent sensor instances.

The two simulated sensor instances were verified independently:

| Property | Sensor 1 | Sensor 2 |

| Device ID | `0x42` | `0x43` |
| Configuration | `0x05` | `0x0A` |
| Temperature | 500 | 600 |
| Temperature (hex) | `0x01F4` | `0x0258` |

**Note:** The SPI sensor devices used in these driver integration tests are simulated in software. Successful execution on the Pico verifies the firmware integration and test logic, not communication with physical external SPI sensors.

## Progress

Completed areas include:

- Embedded C fundamentals.
- Memory and bit manipulation.
- Microcontroller architecture.
- Memory-mapped registers.
- GPIO.
- Timers and interrupts.
- UART communication.
- I2C fundamentals.
- SPI communication.
- Sensor driver development.
- Hardware abstraction.
- Multiple device instances.
- Structured error handling.
- Testing and integration.
