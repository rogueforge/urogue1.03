HDRS=	../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/tunable.h \
	../hdrs/curses.h ../hdrs/varargs.h ../hdrs/score.h \
	../hdrs/stepok.h ../hdrs/death.h

OBJS= 	vers.o monsdata.o magic_items.o armor.o artifact.o chase.o \
	command.o daemon.o bag.o properties.o ident.o \
	daemons.o fight.o encumb.o get_play.o init.o io.o keypad.o list.o \
	main.o maze.o misc.o monsters.o move.o new_level.o options.o pack.o \
	passages.o player.o potions.o rings.o rip.o rooms.o save.o scrolls.o \
	sticks.o things.o trader.o sys_dep.o weapons.o wizard.o rogue.o magic.o

CFILES=	vers.c monsdata.c magic_items.c armor.c artifact.c chase.c \
	command.c daemon.c bag.c properties.c ident.c \
	daemons.c fight.c encumb.c get_play.c init.c io.c keypad.c list.c \
	main.c maze.c misc.c monsters.c move.c new_level.c options.c pack.c \
	passages.c player.c potions.c rings.c rip.c rooms.c save.c scrolls.c \
	sticks.c things.c trader.c sys_dep.c weapons.c wizard.c rogue.c magic.c

# Installation variables.
LIB=		libsrogue.o

CFLAGS= -O -I../hdrs

libsrogue.o:	$(OBJS)
		ld -r -o $(LIB) $(OBJS) 

lint:
		lint -hxbc -I../hdrs $(CFILES) -lcurses

clean:
		rm -f $(OBJS) core $(LIB)

vers.o:		../hdrs/tunable.h
monsdata.o:	../hdrs/rogue.h
magic_items.o:	../hdrs/rogue.h
armor.o:	../hdrs/rogue.h
artifact.o:	../hdrs/rogue.h ../hdrs/death.h
chase.o:	../hdrs/rogue.h ../hdrs/stepok.h
command.o:	../hdrs/rogue.h ../hdrs/death.h
daemon.o:	../hdrs/rogue.h
daemons.o:	../hdrs/rogue.h ../hdrs/stepok.h ../hdrs/death.h
fight.o:	../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/death.h
encumb.o:	../hdrs/rogue.h
get_play.o:	../hdrs/rogue.h
init.o:		../hdrs/rogue.h
io.o:		../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/varargs.h ../hdrs/stepok.h
keypad.o:	../hdrs/rogue.h
list.o:		../hdrs/rogue.h
main.o:		../hdrs/mach_dep.h ../hdrs/rogue.h ../hdrs/tunable.h ../hdrs/score.h
maze.o:		../hdrs/rogue.h
misc.o:		../hdrs/rogue.h
monsters.o:	../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/stepok.h ../hdrs/death.h
move.o:		../hdrs/rogue.h ../hdrs/stepok.h ../hdrs/death.h
new_level.o:	../hdrs/rogue.h
options.o:	../hdrs/rogue.h ../hdrs/mach_dep.h 
pack.o:		../hdrs/rogue.h
passages.o:	../hdrs/rogue.h
player.o:	../hdrs/rogue.h
potions.o:	../hdrs/rogue.h ../hdrs/death.h
rings.o:	../hdrs/rogue.h
rip.o:		../hdrs/rogue.h ../hdrs/mach_dep.h ../hdrs/score.h
rooms.o:	../hdrs/rogue.h
save.o:		../hdrs/rogue.h ../hdrs/mach_dep.h
scrolls.o:	../hdrs/rogue.h ../hdrs/stepok.h
sticks.o:	../hdrs/rogue.h
things.o:	../hdrs/rogue.h
trader.o:	../hdrs/rogue.h
sys_dep.o:	../hdrs/rogue.h ../hdrs/mach_dep.h
weapons.o:	../hdrs/rogue.h
wizard.o:	../hdrs/rogue.h ../hdrs/mach_dep.h
rogue.o:	../hdrs/rogue.h
magic.o:	../hdrs/rogue.h ../hdrs/death.h
