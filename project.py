default.CC="gcc"
default.CFLAGS = "-g -Os -Wall -std=c11"
default.includes += "include"
default.includes += "src/posix"
default.defines += ["LINUX", "_DEFAULT_SOURCE"]
#default.LINK="musl-gcc"
#default.LINKFLAGS="-static"

Main("echo_server", ["echo_server.c", "src/posix/posix.c" ])
Main("echo_client", ["echo_client.c", "src/posix/posix.c" ])

subinclude("ndhcp")
subinclude("nanocoap")

set_clean_leftovers()
