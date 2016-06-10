#ifndef NDHCPC_H
#define NDHCPC_H

#include <stdint.h>

#include "sock_udp.h"

enum {
    DHCP_STATE_DISCOVER,
    DHCP_STATE_REQUEST,
    DHCP_STATE_BOUND
};

typedef struct __attribute__((packed)) {
    uint32_t op_htype_hlen_hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint32_t chaddr[4];
    uint8_t padding[192];
    uint32_t magic_cookie;
    uint8_t hdr_end[];
} dhcp_pkt_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    uint8_t data[];
} dhcp_opt_t;

typedef struct {
    uint32_t addr;
    uint32_t netmask;
    uint32_t gw;
    uint32_t dns;
    uint16_t mtu;
} dhcp_lease_t;

typedef struct {
    unsigned state;
    unsigned iface;
    uint32_t xid;
    sock_udp_t sock;
    unsigned lease_time;
} ndhcpc_t;

int ndhcpc_init(ndhcpc_t *ndhcpc, unsigned iface, uint32_t xid);
int ndhcpc_loop(ndhcpc_t *ndhcpc);

int ndhcpc_get_l2addr(unsigned iface, uint8_t *dst);
void ndhcpc_handle_lease(dhcp_lease_t *lease);

#endif /* NDHCPC_H */
