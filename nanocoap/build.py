default.CFLAGS += "-Wall"
Main("nanocoap/nanocoap", glob.glob("*.c") + ["../src/posix/posix.c"])
