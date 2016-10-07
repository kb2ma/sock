#ifndef SOCK_UTIL_H
#define SOCK_UTIL_H

int sock_udp_fmt_endpoint(const sock_udp_ep_t *endpoint, char *addr_str, uint16_t *port);
int sock_urlsplit(const char *url, char *hostport, char *urlpath);
int sock_str2ep(sock_udp_ep_t *ep_out, const char *str);

#define SOCK_HOSTPORT_MAXLEN    (32U)
#define SOCK_URLPATH_MAXLEN    (32U)

#endif /* SOCK_UTIL_H */
