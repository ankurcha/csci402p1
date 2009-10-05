// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include <string>

#include "copyright.h"
#include "filesys.h"
#include "table.h"
#include "synch.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

#define MaxCV 1024
#define MaxLock 1024

// global map of available physical memory
Lock* physMemMapLock = new Lock("physMemMapLock");
BitMap physMemMap(NumPhysPages);

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    // read a string at the virtual address s
    std::string readCString(char* s);
    
    //TODO: need to support new stacks for multiple threads

    Table fileTable;			// Table of openfiles
    
    Table locksTable;           //Table of Locks
    Table CVTable;              //Table of CVs

 private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
};

#endif // ADDRSPACE_H
