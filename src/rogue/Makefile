HDRS=	../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/tunable.h \
	../hdrs/curses.h ../hdrs/varargs.h

OBJS= 	vers.o monsdata.o armor.o artifact.o chase.o command.o daemon.o \
	daemons.o fight.o encumb.o get_play.o init.o io.o list.o main.o maze.o \
	misc.o monsters.o move.o new_level.o options.o pack.o \
	passages.o player.o potions.o rings.o rip.o rooms.o save.o scrolls.o \
	sticks.o things.o trader.o sys_dep.o weapons.o wizard.o rogue.o 

CFILES=	vers.c monsdata.c armor.c artifact.c chase.c command.c daemon.c \
	daemons.c fight.c encumb.c get_play.c init.c io.c list.c main.c maze.c \
	misc.c monsters.c move.c new_level.c options.c pack.c \
	passages.c player.c potions.c rings.c rip.c rooms.c save.c scrolls.c \
	sticks.c things.c trader.c sys_dep.c weapons.c wizard.c rogue.c 

# Installation variables.
LIB=		libsrogue.o

CFLAGS= -O -I../hdrs
LDFLAGS=

libsrogue.o:	$(HDRS) $(OBJS)
		ld -r -o $(LIB) $(OBJS) 

lint:
		lint -hxbc $(CFILES) -lcurses

clean:
		rm -f $(OBJS) core $(LIB)

vers.o:		../hdrs/tunable.h
monsdata.o:	../hdrs/rogue.h
armor.o:	../hdrs/curses.h ../hdrs/rogue.h
artifact.o:	../hdrs/curses.h ../hdrs/rogue.h
chase.o:	../hdrs/curses.h ../hdrs/rogue.h
command.o:	../hdrs/curses.h ../hdrs/rogue.h
daemon.o:	../hdrs/curses.h ../hdrs/rogue.h
daemons.o:	../hdrs/curses.h ../hdrs/rogue.h
fight.o:	../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h
encumb.o:	../hdrs/curses.h ../hdrs/rogue.h
get_play.o:	../hdrs/curses.h ../hdrs/rogue.h
init.o:		../hdrs/curses.h ../hdrs/rogue.h
io.o:		../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/varargs.h
list.o:		../hdrs/curses.h ../hdrs/rogue.h
main.o:		../hdrs/curses.h ../hdrs/mach_dep.h ../hdrs/rogue.h ../hdrs/tunable.h
maze.o:		../hdrs/curses.h ../hdrs/rogue.h
misc.o:		../hdrs/curses.h ../hdrs/rogue.h
monsters.o:	../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h
move.o:		../hdrs/curses.h ../hdrs/rogue.h
new_level.o:	../hdrs/curses.h ../hdrs/rogue.h
options.o:	../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h 
pack.o:		../hdrs/curses.h ../hdrs/rogue.h
passages.o:	../hdrs/curses.h ../hdrs/rogue.h
player.o:	../hdrs/curses.h ../hdrs/rogue.h
potions.o:	../hdrs/curses.h ../hdrs/rogue.h
rings.o:	../hdrs/curses.h ../hdrs/rogue.h
rip.o:		../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h 
rooms.o:	../hdrs/curses.h ../hdrs/rogue.h
save.o:		../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h
scrolls.o:	../hdrs/curses.h ../hdrs/rogue.h
sticks.o:	../hdrs/curses.h ../hdrs/rogue.h
things.o:	../hdrs/curses.h ../hdrs/rogue.h
trader.o:	../hdrs/curses.h ../hdrs/rogue.h
sys_dep.o:	../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h
weapons.o:	../hdrs/curses.h ../hdrs/rogue.h
wizard.o:	../hdrs/curses.h ../hdrs/rogue.h ../hdrs/mach_dep.h
rogue.o:	../hdrs/curses.h ../hdrs/rogue.h
