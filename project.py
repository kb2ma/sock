default.CC="gcc"
default.CFLAGS = "-g -Os -Wall -std=c11"
default.includes += "include"
default.includes += "src/posix"
default.includes += "%s/sys/include" % os.environ['RIOTBASE']
default.defines += ["LINUX", "_DEFAULT_SOURCE"]
default.defines += "SOCK_HAS_IPV6"
default.defines += "SOCK_HAS_IPV4"
#default.LINK="musl-gcc"
#default.LINKFLAGS="-static"

Main("echo_server", ["echo_server.c", "src/posix/posix.c" ])
Main("echo_client", ["echo_client.c", "src/posix/posix.c" ])

subinclude("ndhcp")
subinclude("nanocoap")

set_clean_leftovers()
