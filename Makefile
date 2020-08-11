LIB = libartibeus-ctrl

OBJECTS = \
	artibeus.o \

DEPS += libio libmsp libspware

override SRC_ROOT = ../../src
override CFLAGS += -I $(SRC_ROOT)/include/$(LIB)

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
