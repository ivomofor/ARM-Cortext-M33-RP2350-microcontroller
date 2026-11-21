#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool (*initialize)(void);

    bool (*read_register)(uint8_t address, uint8_t *value);

    bool (*write_register)(uint8_t address, uint8_t value);

} communication_interface_t;

#endif