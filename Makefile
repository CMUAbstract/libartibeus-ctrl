LIB = libartibeus

OBJECTS = \
	artibeus.o \
  comm.o\
  query.o\
  handle_uarts.o\
  handle_uarts.o\
  backup.o\

override SRC_ROOT = ../../src

override CFLAGS += \
	-I$(SRC_ROOT)/include \
	-I$(SRC_ROOT)/include/$(LIB) \

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
