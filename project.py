default.CFLAGS = "-g -Os -Wall -std=c11 -I."

Main("sender", ["sender.c", "posix.c" ])
Main("receiver", ["receiver.c", "posix.c" ])

subinclude("ndhcp")

set_clean_leftovers()
