# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux PPPoE plugin.
#
# Copyright (C) 2001 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# $Id: //depot/sw/releases/Aquila_9.2.0_U10.5/apps/pppoe/ppp-2.4.4/pppd/plugins/rp-pppoe/Makefile#1 $
#***********************************************************************

DESTDIR = $(INSTROOT)/usr/local
BINDIR = $(DESTDIR)/sbin
LIBDIR = $(DESTDIR)/lib/pppd/$(PPPDVERSION)

PPPDVERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../../patchlevel.h)

INSTALL	= install

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.3

COPTS=-O2 -g
CFLAGS=$(COPTS) -I../../../include/linux
all: rp-pppoe.so pppoe-discovery

pppoe-discovery: libplugin.a pppoe-discovery.o
	$(CC) -o pppoe-discovery pppoe-discovery.o libplugin.a

pppoe-discovery.o: pppoe-discovery.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pppoe-discovery.o pppoe-discovery.c

rp-pppoe.so: libplugin.a plugin.o
	$(CC) -o rp-pppoe.so -shared plugin.o libplugin.a

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -c -m 4550 rp-pppoe.so $(LIBDIR)
#	$(INSTALL) -d -m 755 $(BINDIR)
#	$(INSTALL) -s -c -m 555 pppoe-discovery $(BINDIR)

clean:
	rm -f *.o *.so

plugin.o: plugin.c
	$(CC) '-DRP_VERSION="$(VERSION)"' $(CFLAGS) -I../../..  -c -o plugin.o -fPIC plugin.c

libplugin.a: discovery.o if.o common.o debug.o
	$(AR) -rc $@ $^

discovery.o: discovery.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o discovery.o -fPIC discovery.c

if.o: if.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o if.o -fPIC if.c

debug.o: debug.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o debug.o -fPIC debug.c

common.o: common.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o common.o -fPIC common.c

