#
# File: Makefile
# Project: PS3005D-DC-Lab-Power-Supply
#
# Created by Sven Kreiensen on 28.04.13.
# Copyright (c) 2013 Sven Kreiensen
#
# Usage:
#
# make all = Build and compile software
#
# make clean = Remove als build files
#
# make install = Ropy binary executable to the destination folder
#

OS ?= LINUX
#OS ?= MACOSX

ifeq ($(OS), MACOSX)
CC=gcc
LD=LD
CFLAGS += -DMACOSX
INSTALLDIR=/opt/local/bin
else
# GCC cross compiler prefix
#CROSS=mips-uclibc-
CC=$(CROSS)gcc
LD=$(CROSS)ld
STRIP=$(CROSS)strip
INSTALLDIR=/usr/local/bin
endif

CFLAGS += -s -O2 -DDEBUG -Wall -I.
LDFLAGS = -lc -lm

all: ps3005d_powersupply

clean:
	rm -f *.o *~ ps3005d_powersupply

install:
	cp ps3005d_powersupply $(INSTALLDIR)

ps3005d_powersupply: ps3005d_powersupply.o
	$(CC) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@
