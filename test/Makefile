# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)


all: halt matmult sort testfiles lockcreate destroylock createcv destroycv fork exec exectest printtest queue init test_code multih networkTestClient1 networkTestClient2 multiForkmatmult multiExecmatmult multiExecmatmult2 multiprint


start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

lockcreate.o: lockcreate.c
	$(CC) $(CFLAGS) -c lockcreate.c
lockcreate: lockcreate.o start.o
	$(LD) $(LDFLAGS) start.o lockcreate.o -o lockcreate.coff
	../bin/coff2noff lockcreate.coff lockcreate

destroylock.o: destroylock.c
	$(CC) $(CFLAGS) -c destroylock.c
destroylock: destroylock.o start.o
	$(LD) $(LDFLAGS) start.o destroylock.o -o destroylock.coff
	../bin/coff2noff destroylock.coff destroylock

createcv.o: createcv.c
	$(CC) $(CFLAGS) -c createcv.c
createcv: createcv.o start.o
	$(LD) $(LDFLAGS) start.o createcv.o -o createcv.coff
	../bin/coff2noff createcv.coff createcv
destroycv.o: destroycv.c
	$(CC) $(CFLAGS) -c destroycv.c
destroycv: destroycv.o start.o
	$(LD) $(LDFLAGS) start.o destroycv.o -o destroycv.coff
	../bin/coff2noff destroycv.coff destroycv

acquire.o: acquire.c
	$(CC) $(CFLAGS) -c acquire.c
acquire: acquire.o start.o
	$(LD) $(LDFLAGS) start.o acquire.o -o acquire.coff
	../bin/coff2noff acquire.coff acquire
fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o
	$(LD) $(LDFLAGS) start.o fork.o -o fork.coff
	../bin/coff2noff fork.coff fork
exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec

exectest.o: exectest.c
	$(CC) $(CFLAGS) -c exectest.c
exectest: exectest.o start.o
	$(LD) $(LDFLAGS) start.o exectest.o -o exectest.coff
	../bin/coff2noff exectest.coff exectest

printtest.o: printtest.c
	$(CC) $(CFLAGS) -c printtest.c
printtest: printtest.o start.o
	$(LD) $(LDFLAGS) start.o printtest.o -o printtest.coff
	../bin/coff2noff printtest.coff printtest

print.o: print.c print.h
	$(CC) $(CFLAGS) -c print.c
print: print.o

queue.o: queue.c queue.h print
	$(CC) $(CFLAGS) -c queue.c
queue: queue.o

init.o: init.c print queue
	$(CC) $(CFLAGS) -c init.c
init: init.o start.o
	$(LD) $(LDFLAGS) start.o init.o -o init.coff
	../bin/coff2noff init.coff init
test_code.o: test_code.c
	$(CC) $(CFLAGS) -c test_code.c
test_code: test_code.o start.o
	$(LD) $(LDFLAGS) start.o test_code.o -o test_code.coff
	../bin/coff2noff test_code.coff test_code

multih.o: multih.c
	$(CC) $(CFLAGS) -c multih.c
multih: multih.o start.o
	$(LD) $(LDFLAGS) start.o multih.o -o multih.coff
	../bin/coff2noff multih.coff multih

networkTestClient1.o: networkTestClient1.c print
	$(CC) $(CFLAGS) -c networkTestClient1.c
networkTestClient1: networkTestClient1.o start.o
	$(LD) $(LDFLAGS) start.o networkTestClient1.o -o networkTestClient1.coff
	../bin/coff2noff networkTestClient1.coff networkTestClient1

networkTestClient2.o: networkTestClient2.c print
	$(CC) $(CFLAGS) -c networkTestClient2.c
networkTestClient2: networkTestClient2.o start.o
	$(LD) $(LDFLAGS) start.o networkTestClient2.o -o networkTestClient2.coff
	../bin/coff2noff networkTestClient2.coff networkTestClient2

multiExecmatmult.o: multiExecmatmult.c
	$(CC) $(CFLAGS) -c multiExecmatmult.c
multiExecmatmult: multiExecmatmult.o start.o
	$(LD) $(LDFLAGS) start.o multiExecmatmult.o -o multiExecmatmult.coff
	../bin/coff2noff multiExecmatmult.coff multiExecmatmult

multiExecmatmult2.o: multiExecmatmult2.c
	$(CC) $(CFLAGS) -c multiExecmatmult2.c
multiExecmatmult2: multiExecmatmult2.o start.o
	$(LD) $(LDFLAGS) start.o multiExecmatmult2.o -o multiExecmatmult2.coff
	../bin/coff2noff multiExecmatmult2.coff multiExecmatmult2

multiForkmatmult.o: multiForkmatmult.c
	$(CC) $(CFLAGS) -c multiForkmatmult.c
multiForkmatmult: multiForkmatmult.o start.o
	$(LD) $(LDFLAGS) start.o multiForkmatmult.o -o multiForkmatmult.coff
	../bin/coff2noff multiForkmatmult.coff multiForkmatmult

multiprint.o: multiprint.c print
	$(CC) $(CFLAGS) -c multiprint.c
multiprint: multiprint.o start.o
	$(LD) $(LDFLAGS) start.o print.o multiprint.o -o multiprint.coff
	../bin/coff2noff multiprint.coff multiprint
clean:
	rm -f *.o *.coff

