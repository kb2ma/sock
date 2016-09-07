default.CFLAGS += "-Wall"

common_srcs = [ "nanocoap.c", "handler.c", "../src/posix/posix.c" ]
Main("nanocoap/nanocoap_server", [ "server.c" ] + common_srcs)
Main("nanocoap/nanocoap_client", [ "client.c", "nanocoap_sock.c", "../src/str2ep.c" ] + common_srcs)
