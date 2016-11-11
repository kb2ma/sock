all: bin/nanocoap_client bin/nanocoap_server

CFLAGS += -g -Os -Wall -Wextra -pedantic -std=c11
CFLAGS += -I../include -I../riot/sys/include -I../src/posix

CFLAGS += -DSOCK_HAS_IPV4 -DSOCK_HAS_IPV6 -DLINUX -D_DEFAULT_SOURCE

SHARED_SRC=nanocoap.c handler.c nanocoap_sock.c ../src/util.c ../src/posix/posix.c
CLIENT_SRC=client.c nanocoap_sock.c $(SHARED_SRC)
SERVER_SRC=server.c $(SHARED_SRC)

bin/:
	@mkdir -p bin

bin/nanocoap_client: $(CLIENT_SRC) | bin/
	$(CC) $(CFLAGS) $^ -o $@

bin/nanocoap_server: $(SERVER_SRC) | bin/
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bin/nanocoap_client bin/nanocoap_server
