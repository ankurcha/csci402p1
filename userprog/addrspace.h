// addrspace.h 
//    Data structures to keep track of executing user programs 
//    (address spaces).
//
//    For now, we don't keep any information about address spaces.
//    The user level CPU state is saved and restored in the thread
//    executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include <string>

#include "copyright.h"
#include "filesys.h"
#include "noff.h"
#include "table.h"
#include <set>

using namespace std; // this is bad practice in .h files -max
typedef int PID;
#define UserStackSize   16384  // increase this as necessary!
#define NumVirtPages    8192  // size of the address space

#define MaxOpenFiles 256
#define MaxChildSpaces 256

#define MaxCV 1024
#define MaxLock 1024

// global map of available physical memory
enum status{
    NOWHERE,  // page is not allocated anywhere (can still be valid)
    MEMORY, // in physical memory
    EXEC,  // in the executabe file
    SWAP // in the swap file
};

class PageTableEntry: public TranslationEntry{
    public:
    // Add new Filds to the page table such that it can take care of
    // status and the location of the page.
    status PageStatus;
    int swapLocation;
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);    // Create an address space,
                    // initializing it with the program
                    // stored in the file "executable"
    ~AddrSpace();            // De-allocate an address space

    void InitRegisters();        // Initialize user-level CPU registers,
                    // before jumping to user code

    void SaveState();            // Save/restore address space-specific
    void RestoreState();        // info on a context switch
#ifdef CHANGED
    // create a new stack for a thread and set StackReg there
    int InitStack();

    // clear the stack of an exiting thread
    void ClearStack(int id);
    
    // read a string at the virtual address s
    std::string readCString(char* s);
    
#endif
    // Assume linear page table translation
    // for now! 
    TranslationEntry *pageTable;
 public:
            

    // Number of pages in the virtual address space
    unsigned int numPages;

#ifdef CHANGED
    // number of bytes and pages used by code and data
    unsigned int dataSize;
    unsigned int dataPages;
    OpenFile *executable;
    NoffHeader noffH;
    // keep track of the stacks in this process
    Lock* stackTableLock;
    BitMap* stackTable;
    PageTableEntry *pageTableInfo;

    Table fileTable;            // Table of openfiles

    Lock *locksTableLock;
    Table locksTable;           //Table of Locks
    Table CVTable;              //Table of CVs
    Lock *CVTableLock;
    int childThreads;        // PID of Children Threads
#endif
};

#endif // ADDRSPACE_H
