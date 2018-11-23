all: bin/nanocoap_client bin/nanocoap_server

CFLAGS += -g -Os -Wall -Wextra -pedantic -std=c11
CFLAGS += -I../include -I../riot/sys/include -I../src/posix/include

CFLAGS += -DSOCK_HAS_IPV4 -DSOCK_HAS_IPV6 -DLINUX -D_DEFAULT_SOURCE

RIOT_SOCK=../riot/sys/net/application_layer/nanocoap
RIOT_FMT=../riot/sys/fmt
SHARED_SRC=${RIOT_FMT}/fmt.c ${RIOT_SOCK}/nanocoap.c ${RIOT_SOCK}/sock.c ../riot/sys/net/sock/sock_util.c ../src/posix/posix.c handler.c
CLIENT_SRC=client.c $(SHARED_SRC)
SERVER_SRC=server.c $(SHARED_SRC)

bin/:
	@mkdir -p bin

bin/nanocoap_client: $(CLIENT_SRC) | bin/
	$(CC) $(CFLAGS) $^ -o $@

bin/nanocoap_server: $(SERVER_SRC) | bin/
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bin/nanocoap_client bin/nanocoap_server
