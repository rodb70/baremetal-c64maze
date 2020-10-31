#CPU := c64
CPU := host

CSRC += c64maze.c
ifeq ($(CPU),c64)
COMPILER := cc65
endif
ifeq ($(CPU),host)
COMPILER := gcc
SHORT_ENUMS := n
endif

CFLAGS += -I .

BLD_TARGET := c64maze
BLD_TYPE := debug

PROJ_DIRS += $(CPU)

include makefiles/main.mk

distclean:
	rm -rf build

