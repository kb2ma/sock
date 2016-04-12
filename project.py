default.CC="gcc"
default.CFLAGS = "-g -Os -Wall -std=c11 -I."
#default.LINK="musl-gcc"
#default.LINKFLAGS="-static"

Main("echo_server", ["echo_server.c", "posix.c" ])
Main("echo_client", ["echo_client.c", "posix.c" ])

subinclude("ndhcp")
subinclude("nanocoap")

set_clean_leftovers()
