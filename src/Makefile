DISTRIBDIR	= /usr/herbie/tmp
INSTALLDIR	= /usr/herbie/bin
CURSES		= -lcurses
TERMCAP		=

urogue: library
	cc commands/tunable.o rogue/libsrogue.o -o urogue $(CURSES) $(TERMCAP)
	@chmod 711 urogue
	@ls -l urogue
	@size urogue

install: urogue
	@-mkdir $(INSTALLDIR)
	cp commands/control $(INSTALLDIR)
	cp urogue $(INSTALLDIR)
	chmod 4711 $(INSTALLDIR)/urogue

distrib: library
	@-mkdir $(DISTRIBDIR)
	@chmod 755 $(DISTRIBDIR)
	@-rm -f $(DISTRIBDIR)/*
	cp distfiles/Makefile.INST $(DISTRIBDIR)/Makefile
	cp distfiles/README commands/tunable.c commands/control.o $(DISTRIBDIR)
	cp rogue/libsrogue.o $(DISTRIBDIR)
	@chmod 644 $(DISTRIBDIR)/*

library: control
	cd hdrs; make rogue.h
	cd rogue ; make libsrogue.o

control:
	cd commands; make control

print: commands/print_a commands/print_i commands/print_m commands/print_w
	cd commands; make print

clean:
	cd commands; make clean
	cd rogue; make clean
	rm -f core urogue

tags:
	cd rogue; make tags

lint:
	cd rogue; make lint
