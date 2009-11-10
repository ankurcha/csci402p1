// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "processTable.h"

#ifdef CHANGED
using namespace std;
typedef int PID;

class InvertedPageTableEntry : public TranslationEntry{
public:
    InvertedPageTableEntry(){
            virtualPage = -1;
            physicalPage = -1;
            valid = false;
            dirty = false;
        }
    enum {
            CODE,
            DATA,
            OTHER
        } ContentType;
    enum {
            MEMORY,
            EXEC,
            SWAP,
        }Location;

    int PID;    // TO find out who is the owner
    int age;    // Used for FIFO replacement
    AddrSpace *space; // To keep track of which pages we are refering to.
    status PageStatus;
    int swapLocation;
};

#endif
// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef CHANGED

#ifdef USE_TLB
extern InvertedPageTableEntry *IPT;
extern Lock* IPTLock;
extern Lock* TLBIndexLock;
extern bool FIFOreplacementPolicy;
extern OpenFile *swapFile;
//extern int swapLocation;
extern Lock* swapLock;
extern BitMap *swapBitMap;
#endif

#endif

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
extern ProcessTable *processTable;  // Process Table for Nachos
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
extern int netname;
#endif

#endif // SYSTEM_H
