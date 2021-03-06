# Makefile.conf
#
# Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
# Copyright (c) 1995-2003 by Internet Software Consortium
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#   Internet Systems Consortium, Inc.
#   950 Charter Street
#   Redwood City, CA 94063
#   <info@isc.org>
#   http://www.isc.org/

## Defaults...
SCRIPT = none
USERBINDIR = /usr/bin
BINDIR = /usr/sbin
CLIENTBINDIR=/sbin
ADMMANDIR = /usr/share/man/cat8
ADMMANEXT = .0
FFMANDIR = /usr/share/man/cat5
FFMANEXT = .0
LIBMANDIR = /usr/share/man/cat3
LIBMANEXT = .0
USRMANDIR = /usr/share/man/cat1
USRMANEXT = .0
MANCAT = cat
INSTALL = install -c -m 444
MANINSTALL = install -c
CHMOD = chmod
CATMANPAGES =
ETC = /etc
VARRUN = /var/run
VARDB = /var/db
LIBDIR=/usr/local/lib
INCDIR=/usr/local/include
LIBS =
COPTS = $(BINDDEF) $(CC_OPTIONS)
DEBUG = -g
#WARNERR = -Werror
MKDEP = mkdep
CLIENT_PATH = '"PATH=/usr/ucb:/usr/bin:/usr/sbin:/bin:/sbin"'

BINDLIB = ../minires/libres.a
BINDINC =
MINIRES = minires
TOUCH = touch

# Major version number (if applicable)
##--majver--
MAJORVERSION=2
##--majver--

# Minor version number (if applicable)
##--minver--
MINORVERSION=6
##--minver--

## Porting::
#
# For each supported operating system, there is a block of text below
# beginning with #--os-name-- and ending with #--os-name--.   Between
# these delimiters are assignments, commented out, which define the
# Makefile variables required for that operating system.
#
# The configure shell script figures out what operating system it's
# being run on and then runs Makefile.dist through a sed script which
# removes the comment characters from the appropriate set of
# assignments, and writes the output to Makefile.

## AIX 4.3
##--aix--
#CF = cf/aix.h
#CC=cc -Daix
#INSTALL=/usr/ucb/install -c
#MANINSTALL=/usr/ucb/install -c -m 444
#ADMMANEXT = .8
#FFMANEXT = .5
#VARRUN = /etc
#VARDB = /etc
##--aix--

## NEXTSTEP 3.x,4.x
##--nextstep--
#CF = cf/nextstep.h
#CC=cc
#COPTS = -Wall $(BINDDEF) $(CC_OPTIONS)
#BINDIR=/usr/etc
#ADMMANDIR = /usr/local/man/cat8
#FFMANDIR = /usr/local/man/cat5
#LIBMANDIR = /usr/local/man/cat3
#USRMANDIR = /usr/local/man/cat1
#ADMMANEXT = .8
#FFMANEXT = .5
#LIBMANEXT = .3
#USRMANEXT = .3
#VARRUN = /etc
#VARDB = /etc
##--nextstep--

## SunOS 4.1
##--sunos4--
#CC = gcc
#LIBS = -lresolv
#CF = cf/sunos4.h
#BINDIR=/usr/etc
#CLIENTBINDIR=/etc
#ADMMANEXT = .8
#FFMANEXT = .5
#VARRUN = /etc
#VARDB = /etc
##--sunos4--

## Solaris 2.5 (with gcc)
##--sunos5-gcc--
#INSTALL=/usr/ucb/install
#MANINSTALL=/usr/ucb/install
#LIBS = -lresolv -lsocket -lnsl -lgen
#CC=gcc
#COPTS = $(BINDDEF) -Wall -Wno-unused -Wno-implicit -Wno-comment \
#	 -Wno-uninitialized -Wno-char-subscripts -Wno-switch $(WARNERR) \
#	 -DSOLARIS_MAJOR=$(MAJORVERSION) -DSOLARIS_MINOR=$(MINORVERSION) \
#	 $(CC_OPTIONS)
#CF = cf/sunos5-5.h
#ADMMANDIR = /usr/share/man/man1m
#ADMMANEXT = .1m
#FFMANDIR = /usr/share/man/man4
#FFMANEXT = .4
#LIBMANDIR = /usr/share/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/share/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /etc
#VARDB = /etc
#SCRIPT=solaris
##--sunos5-gcc--

## Solaris 2.5 (with Sun cc)
##--sunos5-cc--
#INSTALL=/usr/ucb/install
#MANINSTALL=/usr/ucb/install
#LIBS = -lresolv -lsocket -lnsl -lgen 
#CC=cc
#COPTS = -D__svr4__ $(BINDDEF) -erroff=E_END_OF_LOOP_CODE_NOT_REACHED \
#	 -DSOLARIS_MAJOR=$(MAJORVERSION) -DSOLARIS_MINOR=$(MINORVERSION) \
#	 $(CC_OPTIONS)
#CF = cf/sunos5-5.h
#ADMMANDIR = /usr/share/man/man1m
#ADMMANEXT = .1m
#FFMANDIR = /usr/share/man/man4
#FFMANEXT = .4
#LIBMANDIR = /usr/share/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/share/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /etc
#VARDB = /etc
#SCRIPT=solaris
##--sunos5-cc--

## DEC Alpha/OSF1
##--alphaosf--
#COPTS = -std $(CC_OPTIONS)
#INSTALL=/usr/ucb/installbsd -c
#MANINSTALL=/usr/ucb/installbsd -c
#CF = cf/alphaosf.h
#ADMMANEXT = .8
#FFMANEXT = .5
#VARDB = /etc
##--alphaosf--

## BSD/OS 2.1 through 4.0
##--bsdos--
#LIBS = -lresolv
#CC=gcc2
#CF = cf/bsdos.h
#SCRIPT=bsdos
##--bsdos--

## BSD/OS 4.1 and higher.
##--bsdos-4.1--
#CF = cf/bsdos.h
#SCRIPT=bsdos
##--bsdos-4.1--

## OpenBSD
##--openbsd--
#CF = cf/openbsd.h
#SCRIPT=openbsd
##--openbsd--

## FreeBSD
##--freebsd--
#CF = cf/freebsd.h
#COPTS = -O -Wall -Wno-unused $(WARNERR) $(CC_OPTIONS)
#SCRIPT=freebsd
##--freebsd--

## Rhapsody
##--rhapsody--
#CF = cf/rhapsody.h
#COPTS  = -Wall -Wno-unused -Wno-implicit -Wno-comment \
#	  -Wno-uninitialized -Wno-switch $(WARNERR) -pipe $(BINDDEF) $(CC_OPTIONS)
##SCRIPT=rhapsody
##--rhapsody--

## Darwin/MacOSX
##--darwin--
#CF = cf/rhapsody.h
#COPTS  = -Ddarwin -Wall -Wno-unused -Wno-implicit -Wno-comment \
#	  -Wno-uninitialized -Wno-switch $(WARNERR) -pipe $(BINDDEF) $(CC_OPTIONS)
#SCRIPT=freebsd
##--darwin--

## NetBSD
##--netbsd--
#CF = cf/netbsd.h
#COPTS  = -Wall -Wstrict-prototypes -Wno-unused -Wno-comment \
#	  -Wno-uninitialized $(WARNERR) \
#	  -Wimplicit-function-declaration -Wpointer-arith \
#	  -Wwrite-strings -Wnested-externs \
#	  -pipe $(BINDDEF) $(CC_OPTIONS)
#SCRIPT=netbsd
##--netbsd--

## Ultrix
##--ultrix--
#BINDIR = /usr/etc
#CLIENTBINDIR=/etc
#VARRUN = /etc
#VARDB = /etc
#CF = cf/ultrix.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/cat3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/cat1
#USRMANEXT = .1
##--ultrix--

## Linux 1.x
##--linux-1--
#COPTS = -DLINUX_MAJOR=$(MAJORVERSION) -DLINUX_MINOR=$(MINORVERSION) \
#        $(BINDDEF) $(CC_OPTIONS)
#CF = cf/linux.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /var/run
#VARDB = /var/state/dhcp
#SCRIPT=linux
##--linux-1--

## Linux 2.0
##--linux-2.0--
#COPTS = -DLINUX_MAJOR=$(MAJORVERSION) -DLINUX_MINOR=$(MINORVERSION) \
#        $(BINDDEF) $(CC_OPTIONS)
#CF = cf/linux.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /var/run
#VARDB = /var/state/dhcp
#SCRIPT=linux
##--linux-2.0--

## Linux 2.1
##--linux-2.1--
#COPTS = -DLINUX_MAJOR=$(MAJORVERSION) -DLINUX_MINOR=$(MINORVERSION) \
#        $(BINDDEF) $(CC_OPTIONS)
#CF = cf/linux.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /var/run
#VARDB = /var/state/dhcp
#SCRIPT=linux
##--linux-2.1--

## Linux 2.2
#--linux-2.2--
COPTS = -DLINUX_MAJOR=$(MAJORVERSION) -DLINUX_MINOR=$(MINORVERSION) \
        $(BINDDEF) $(CC_OPTIONS)
CF = cf/linux.h
ADMMANDIR = /usr/man/man8
ADMMANEXT = .8
FFMANDIR = /usr/man/man5
FFMANEXT = .5
LIBMANDIR = /usr/man/man3
LIBMANEXT = .3
USRMANDIR = /usr/man/man1
USRMANEXT = .1
MANCAT = man
VARRUN = /var/run
VARDB = /var/state/dhcp
SCRIPT=linux
#--linux-2.2--

## SCO UnixWare 7
##--uw7--
#CF = cf/uw7.h
#PREDEFINES=-DUW7
#RANLIB=true
#LIBS = -lresolv -lsocket -lnsl -lgen
##--uw7--

## SCO (with standard cc compiler)
##--sco-cc--
#CF = cf/sco.h
#PREDEFINES=-DSCO -DBROKEN_ANSI
#BINDIR = /usr/etc
#CLIENTBINDIR=/etc
#ADMMANDIR = /usr/man/cat.ADMN
#ADMMANEXT = .ADMN.Z
#FFMANDIR = /usr/man/cat.SFF
#FFMANEXT = .SFF.Z
#INSTALL = cp
#MANFROM = <
#MANINSTALL = compress
#MANTO = >
#VARRUN = /etc
#VARDB = /etc
#CATMANPAGES=
##--sco-cc--

## SCO (with GNU gcc compiler)
##--sco-gcc--
#CF = cf/sco.h
#CC=gcc
#PREDEFINES=-DSCO
#LIBS = -lsocket
#USERBINDIR = /usr/local/dhcp/bin
#BINDIR = /usr/local/dhcp/bin
#CLIENTBINDIR = /usr/local/dhcp/bin
#ADMMANDIR = /usr/local/dhcp/man/cat.ADMN
#ADMMANEXT = .ADMN.Z
#FFMANDIR = /usr/local/dhcp/man/cat.SFF
#FFMANEXT = .SFF.Z
#INSTALL = /usr/local/bin/install
#MANFROM = <
#MANINSTALL = compress
#MANTO = >
#VARRUN = /usr/local/dhcp/var
#VARDB = /usr/local/dhcp/var
#CATMANPAGES=
#CLIENT_PATH = '"PATH=/bin:/etc:/usr/bin:/tcb/bin:/usr/bin/X11:/usr/local/bin:/usr/local/dhcp/bin"'
##--sco-gcc--

## QNX
##--qnx--
#CF = cf/qnx.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/man1
#USRMANEXT = .1
#MANCAT = man
#VARRUN = /etc
#COPTS=-w3 -Dlint $(BINDDEF) $(CC_OPTIONS)
#LFLAGS=$(DEBUG) "-Wl,op symfile" -l socket
#MANINSTALL = /bin/true 
#INSTALL = cp
#BINDIR = /etc
#CLIENTBINDIR = /etc
##--qnx--


## QNX RTP (v6, NTO)
##--qnxnto--
#CF = cf/qnx.h
#ADMMANDIR = /opt/man/man8
#ADMMANEXT = .8
#FFMANDIR = /opt/man/man5
#FFMANEXT = .5
#LIBMANDIR = /opt/man/man3
#LIBMANEXT = .3
#MANCAT = man
#COPTS=-w3 -Dlint $(BINDDEF)
#LFLAGS=-l socket
#MANINSTALL = /bin/cp
#INSTALL = /bin/cp
#BINDIR = /opt/sbin
#USERBINDIR= /opt/bin
#CLIENTBINDIR = /opt/sbin
##--qnxnto--

## CygWin32
##--cygwin32--
#CF = cf/cygwin32.h
#ADMMANDIR = /usr/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/man/man1
#USRMANEXT = .1
#VARRUN = /etc
#MANINSTALL = /bin/true 
#INSTALL = cp
#BINDIR = /etc
#CLIENTBINDIR = /etc
#CC=/usr/local/i386-unknown-cygwin32/bin/gcc
#AR=/usr/local/i386-unknown-cygwin32/bin/ar
#AS=/usr/local/i386-unknown-cygwin32/bin/as
#LD=/usr/local/i386-unknown-cygwin32/bin/ld
#NM=/usr/local/i386-unknown-cygwin32/bin/nm
#RANLIB=/usr/local/i386-unknown-cygwin32/bin/ranlib
#STRIP=/usr/local/i386-unknown-cygwin32/bin/strip
##--cygwin32--

## IRIX 6.x
##--irix--
#LFLAGS=$(DEBUG) -Wl,-woff,84 -Wl,-woff,85 -Wl,-woff,134
#CC=gcc
#COPTS = -I/usr/local/include $(BINDDEF) $(CC_OPTIONS)
#CF = cf/irix.h
#BINDIR = /usr/local/etc
#ADMMANDIR = /usr/local/man/man8
#ADMMANEXT = .8
#FFMANDIR = /usr/local/man/man5
#FFMANEXT = .5
#LIBMANDIR = /usr/local/man/man3
#LIBMANEXT = .3
#USRMANDIR = /usr/local/man/man1
#USRMANEXT = .1
#MANCAT = man
#INSTALL = install
#MANINSTALL = install
#CHMOD = chmod
#ETC = /etc
#VARRUN = /etc
#VARDB = /usr/local/etc/dhcp
##--irix--

## HP-UX
##--hpux-cc--
#COPTS = $(BINDDEF) $(CC_OPTIONS)
#LFLAGS = -Wl,+vnocompatwarnings
#INSTALL = install -i
#MANINSTALL = install -i
##--hpux-cc--

## HP-UX with gcc
##--hpux-gcc--
#CC = gcc
#CF = cf/hpux.h
#INSTALL = install -i
#ADMMANEXT = .8
#FFMANEXT = .5
#MANINSTALL = install -i -m 444
##--hpux-gcc--
# Put local site configuration stuff here to override the default
# settings in Makefile.conf
TOP = ../../../dhcp-3.1.0
CC_OPTIONS = -Os
# Makefile.dist
#
# Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#   Internet Systems Consortium, Inc.
#   950 Charter Street
#   Redwood City, CA 94063
#   <info@isc.org>
#   http://www.isc.org/

CATMANPAGES = dhcp-options.cat5 dhcp-eval.cat5
SEDMANPAGES = dhcp-options.man5 dhcp-eval.man5
SRC    = raw.c parse.c nit.c icmp.c dispatch.c conflex.c upf.c bpf.c socket.c \
	 lpf.c dlpi.c packet.c tr.c ethernet.c memory.c print.c options.c \
	 inet.c tree.c tables.c alloc.c fddi.c ctrace.c dns.c resolv.c \
	 execute.c discover.c comapi.c
OBJ    = raw.o parse.o nit.o icmp.o dispatch.o conflex.o upf.o bpf.o socket.o \
	 lpf.o dlpi.o packet.o tr.o ethernet.o memory.o print.o options.o \
	 inet.o tree.o tables.o alloc.o fddi.o ctrace.o dns.o resolv.o \
	 execute.o discover.o comapi.o
MAN    = dhcp-options.5 dhcp-eval.5

INCLUDES = -I$(TOP) $(BINDINC) -I$(TOP)/includes
CFLAGS = $(PREDEFINES) $(INCLUDES) $(COPTS)

all:	libdhcp.a

libdhcp.a:	$(OBJ)
	rm -f libdhcp.a
	$(AR) cruv libdhcp.a $(OBJ)
	$(RANLIB) libdhcp.a

install: all
	for dir in $(FFMANDIR); do \
	  foo=""; \
	  for bar in `echo $(DESTDIR)$${dir} |tr / ' '`; do \
	    foo=$${foo}/$$bar; \
	    if [ ! -d $$foo ]; then \
	      mkdir $$foo; \
	      chmod 755 $$foo; \
	    fi; \
	  done; \
	done
	$(MANINSTALL) $(MANFROM) dhcp-options.$(MANCAT)5 $(MANTO) \
				$(DESTDIR)$(FFMANDIR)/dhcp-options$(FFMANEXT)
	$(MANINSTALL) $(MANFROM) dhcp-eval.$(MANCAT)5 $(MANTO) \
				$(DESTDIR)$(FFMANDIR)/dhcp-eval$(FFMANEXT)

depend:
	$(MKDEP) $(INCLUDES) $(PREDEFINES) $(SRC)

clean:
	-rm -f $(OBJ)

realclean: clean
	-rm -f libdhcp.a $(CATMANPAGES) $(SEDMANPAGES) *~ #*

distclean: realclean
	-rm -f Makefile

links:
	@for foo in $(SRC) $(MAN); do \
	  if [ ! -b $$foo ]; then \
	    rm -f $$foo; \
	  fi; \
	  cp $(TOP)/common/$$foo $$foo; \
	done

dhcp-options.cat5:	dhcp-options.man5
	nroff -man dhcp-options.man5 >dhcp-options.cat5

dhcp-options.man5:	dhcp-options.5
	sed -e "s#ETCDIR#$(ETC)#g" -e "s#DBDIR#$(VARDB)#g" \
		-e "s#RUNDIR#$(VARRUN)#g" < dhcp-options.5 >dhcp-options.man5

dhcp-eval.cat5:	dhcp-eval.man5
	nroff -man dhcp-eval.man5 >dhcp-eval.cat5

dhcp-eval.man5:	dhcp-eval.5
	sed -e "s#ETCDIR#$(ETC)#g" -e "s#DBDIR#$(VARDB)#g" \
		-e "s#RUNDIR#$(VARRUN)#g" < dhcp-eval.5 >dhcp-eval.man5

# Dependencies (semi-automatically-generated)
