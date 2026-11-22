#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    void *context;

    bool (*initialize)(void *context);

    bool (*read_register)(void *context,
                          uint8_t address,
                          uint8_t *value);

    bool (*write_register)(void *context,
                           uint8_t address,
                           uint8_t value);

} communication_interface_t;

#endif