DISTRIBDIR	= /usr/tmp
INSTALLDIR	= /usr/games
CURSES		= -lcurses42
TERMCAP		= -ltermcap
MAKE		= make

urogue: library
	cc commands/tunable.o rogue/libsrogue.o -o urogue $(CURSES) $(TERMCAP) 
	@chmod 711 urogue
	@ls -l urogue
	@size urogue

install: urogue urprint
	@-mkdir $(INSTALLDIR)
	cp commands/control commands/urprint $(INSTALLDIR)
	chmod 711 $(INSTALLDIR)/control $(INSTALLDIR)/urprint
	cp urogue $(INSTALLDIR)
	chmod 4711 $(INSTALLDIR)/urogue

distrib: library
	@-mkdir $(DISTRIBDIR)
	@chmod 755 $(DISTRIBDIR)
	@-rm -f $(DISTRIBDIR)/*
	cp distfiles/Makefile.INST $(DISTRIBDIR)/Makefile
	cp distfiles/README commands/tunable.c commands/control.o $(DISTRIBDIR)
	cp distfiles/tunable.h distfiles/urogue.6 $(DISTRIBDIR)
	cp rogue/libsrogue.o $(DISTRIBDIR)
	@chmod 644 $(DISTRIBDIR)/*

library: control
	cd hdrs; $(MAKE) rogue.h
	cd rogue ; $(MAKE) libsrogue.o

control:
	cd commands; $(MAKE) control

urprint: 
	cd commands; $(MAKE) urprint

clean:
	cd commands; $(MAKE) clean
	cd rogue; $(MAKE) clean
	rm -f core urogue

tags:
	cd rogue; $(MAKE) tags

lint:
	cd rogue; $(MAKE) lint
