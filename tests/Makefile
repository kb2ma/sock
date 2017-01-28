CC = gcc
CFLAGS = -O
INCLUDES = -I.. -I../riot/sys/embunit -I../riot/sys/include
LIBS = embunit
RM = rm
TARGET = tests
NANO_DIR = ../nanocoap/bin
NANO_OBJS =  $(NANO_DIR)/nanocoap.o $(NANO_DIR)/handler.o
OBJS = AllTests.o nanocoapTest.o
RIOTBASE = $(CURDIR)/../riot

all: $(TARGET)

$(TARGET): nano_objs $(OBJS)
	$(CC) -o $@ $(OBJS) $(NANO_OBJS) -L$(LIBS) -lembUnit

.c.o:
	$(CC) \
	-DRIOT_FILE_RELATIVE=\"$(patsubst $(RIOTBASE)/%,%,$(abspath $<))\" \
	$(CFLAGS) $(INCLUDES) -c $<

nano_objs:
	-@cd ../nanocoap;$(MAKE) objs

clean:
	-$(RM) -f $(TARGET) $(OBJS)

.PHONY: clean all nano_objs
