CFLAGS = -O -I../hdrs

#
# Note:  
#	urprint can be only done after monsdata.o and rogue.o have been made
#

control: control.o tunable.o
	cc control.o tunable.o -o control

urprint: urprint.o ../rogue/monsdata.o ../rogue/rogue.o
	cc urprint.o ../rogue/monsdata.o ../rogue/magic_items.o ../rogue/rogue.o -o urprint

clean:
	rm -f core *.o control urprint

control.o: ../hdrs/tunable.h
tunable.o: ../hdrs/tunable.h
