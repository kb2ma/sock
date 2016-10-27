#ifndef NANOCOAP_SOCK_H
#define NANOCOAP_SOCK_H

#include <stdint.h>
#include <unistd.h>

#include "net/sock/udp.h"

int nanocoap_server(sock_udp_ep_t *local, uint8_t *buf, size_t bufsize);

#endif /* NANOCOAP_SOCK_H */
