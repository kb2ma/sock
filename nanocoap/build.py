default.CFLAGS += "-Wall"
Main("nanocoap/nanocoap", glob.glob("*.c") + ["../posix.c"])
