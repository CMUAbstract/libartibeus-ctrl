LIB = libartibeus-ctrl

OBJECTS = \
	artibeus.o \


override SRC_ROOT = ../../src
override CFLAGS += -I $(SRC_ROOT)/include/$(LIB)

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
