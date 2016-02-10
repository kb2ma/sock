#ifndef ADDR_H
#define ADDR_H

#include <stdint.h>

typedef struct {
    uint8_t addr[16];
} ipv6_addr_t;

typedef uint32_t ipv4_addr_t;

#endif /* ADDR_H */
