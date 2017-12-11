all: bin/dns_test

CFLAGS += -g -Os -Wall -Wextra -pedantic -std=c11
CFLAGS += -I../include -I../riot/sys/include -I../src/posix/include

CFLAGS += -DSOCK_HAS_IPV4 -DSOCK_HAS_IPV6 -DLINUX -D_DEFAULT_SOURCE

bin/:
	@mkdir -p bin

bin/dns_test: ../riot/sys/net/application_layer/dns/dns.c ../src/posix/posix.c dns_test.c | bin/
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bin/dns_test
