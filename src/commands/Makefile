CFLAGS = -O -I../hdrs

#
# Note:  
#	print can be only done after monsdata.o and rogue.o have been made
#

control: control.o tunable.o
	cc control.o tunable.o -o control

print: print_a print_i print_m print_w

print_m: print_m.o ../rogue/monsdata.o
	cc print_m.o ../rogue/monsdata.o -o print_m

print_w: print_w.o ../rogue/rogue.o
	cc print_w.o ../rogue/rogue.o -o print_w

print_a: print_a.o ../rogue/rogue.o
	cc print_a.o ../rogue/rogue.o -o print_a

print_i: print_a.o ../rogue/rogue.o
	cc print_i.o ../rogue/rogue.o -o print_i

clean:
	rm -f core *.o control print*.o

control.o: ../hdrs/tunable.h
tunable.o: ../hdrs/tunable.h
