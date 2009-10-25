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

#ifdef CHANGED
#include <map>

using namespace std;
typedef int PID;

struct ProcessTable {
public:
    int processCounter;
    ProcessTable();
    ~ProcessTable();
};

class InvertedPageTableEntry : public TranslationEntry{
    public:
        int PID;    // TO find out who is the owner
        int age;    // Used for FIFO replacement

};

struct eqIPTEntry
{
    bool operator()(int vaddr1, int vaddr2) const{
        return vaddr1 == vaddr2;
    }
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

extern ProcessTable *processTable;  // Process Table for Nachos
#ifdef USE_TLB
//extern hash_map<int, InvertedPageTableEntry, eqIPTEntry> IPT;
extern InvertedPageTableEntry IPT[NumPhysPages];
extern InvertedPageTableEntry Swap[NumPhysPages];
extern BitMap IPTFreePageBitmap;
extern BitMap IPTvalidPageBitmap;
extern bool FIFOreplacementPolicy = false;
extern OpenFile *swapFile;
extern BitMap *swapBitMap;
#endif

#endif

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
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
#endif

#endif // SYSTEM_H
