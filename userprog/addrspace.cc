// addrspace.cc 
//    Routines to manage address spaces (executing user programs).
//
//    In order to run a user program, you must:
//
//    1. link with the -N -T 0 option 
//    2. run coff2noff to convert the object file to Nachos format
//        (Nachos object code format is essentially just a simpler
//        version of the UNIX executable object code format)
//    3. load the NOFF file into the Nachos file system
//        (if you haven't implemented the file system yet, you
//        don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifdef CHANGED
#include <algorithm>
#include <iostream>
#endif

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

#ifdef CHANGED
Lock* physMemMapLock = new Lock("physMemMapLock");
BitMap physMemMap(NumPhysPages);
Lock* childLock;
#endif

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
        delete table;
        table = 0;
    }
    if (lock) {
        delete lock;
        lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;    // to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
        table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
        lock->Acquire();
        if ( map.Test(i) ) {
            map.Clear(i);
            f = table[i];
            table[i] = 0;
        }
        lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
        noffH->noffMagic = WordToHost(noffH->noffMagic);
        noffH->code.size = WordToHost(noffH->code.size);
        noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
        noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
        noffH->initData.size = WordToHost(noffH->initData.size);
        noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
        noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
        noffH->uninitData.size = WordToHost(noffH->uninitData.size);
        noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
        noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

#ifdef CHANGED
AddrSpace::AddrSpace(OpenFile *exec) : fileTable(MaxOpenFiles), 
                                             locksTable(MaxLock), 
                                             CVTable(MaxCV) {
    locksTableLock = new Lock("LocksTableLock");
    CVTableLock = new Lock("CVTableLock");
    //NoffHeader noffH;
    unsigned int i, size, neededPages;
    executable = exec;
    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);
                                                 
    // read the header into noffH
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

    // switch to big endian if it is not already
    if ((noffH.noffMagic != NOFFMAGIC) && 
                (WordToHost(noffH.noffMagic) == NOFFMAGIC))
            SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    dataSize = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    dataPages = divRoundUp(dataSize, PageSize);
    neededPages = dataPages + divRoundUp(UserStackSize,PageSize);
                                                // we need to increase the size
                                                // to leave room for the stack
#ifndef USE_TLB
    ASSERT(neededPages <= NumPhysPages);           // check we're not trying
                                                // to run anything too big --
                                                // at least until we have
                                                // virtual memory
#endif    
    DEBUG('a', "Initializing address space with %d valid pages, size %d\n",
          neededPages, neededPages*PageSize);
    
    // set the stackTable to hold the number of stacks that may exist
    stackTableLock = new Lock("StackTableLock");
    stackTable = new BitMap((NumPhysPages - dataPages) 
                            / divRoundUp(UserStackSize, PageSize));

    // the first stack is in position 0
    //stackTable.push_back((char) true);
    stackTable->Mark(0);

    // and its stack sits in the last pages of the address space
    unsigned int stackStart = 
            NumPhysPages - divRoundUp(UserStackSize, PageSize);
    
        //Take care of the number of child processes
    //childLock = new Lock("childLock");
    this->childThreads = 0;
    
    // first, set up the translation
    numPages = NumPhysPages;
    size = numPages * PageSize;
                                                     // TURN OFF ALL PRELOADING
    DEBUG('a', "Initializing page table, num pages %d, size %d\n", 
                                        numPages, size);
    pageTable = new TranslationEntry[numPages];
    PageTableInfo = new PageTableEntry[numPages]; 

    // allocate physical memory for the pages we are using,
    //  mark the others invalid
    for (i = 0; i < dataPages; i++) {
        // find a free page in physical memory
        physMemMapLock->Acquire();
        int physPage = physMemMap.Find();
        ASSERT(physPage != -1); // make sure a page was found
        physMemMapLock->Release();
#ifndef USE_TLB
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
                                        // a separate page, we could set its 
                                        // pages to be read-only

        // zero out the physical memory associated with this page
        bzero(&(machine->mainMemory[PageSize * physPage]), PageSize);
#endif
    }
    // set all the unused pages to invalid
    for(;i < stackStart; i++) {
#ifndef USE_TLB
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = 0;
        pageTable[i].valid = FALSE;  // no physical memory for this page
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
#endif
    }
    // allocate pages for the stack
    for (; i < NumPhysPages; i++) {
        // find a free page in physical memory
        physMemMapLock->Acquire();
        int physPage = physMemMap.Find();
        ASSERT(physPage != -1); // make sure a page was found
        physMemMapLock->Release();
#ifndef USE_TLB
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
                                        // a separate page, we could set its 
                                        // pages to be read-only

        // zero out the physical memory associated with this page
        bzero(&(machine->mainMemory[PageSize * physPage]), PageSize);
#endif
    }
    
    // then, copy in the code and data segments into memory
#ifndef USE_TLB
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at vaddr 0x%x, size %d\n", 
                        noffH.code.virtualAddr, noffH.code.size);

        // initialize the code segment one page at a time
        int page = noffH.code.virtualAddr / PageSize;
        int offset = noffH.code.virtualAddr % PageSize; // only !=0 for page 1
        int fileAddr = noffH.code.inFileAddr;
        int _size = -1;
        for( int code = noffH.code.size; code > 0; code -= _size) {
            ASSERT(pageTable[page].valid);
            int paddr = (pageTable[page].physicalPage * PageSize) + offset;
            _size = min(code, PageSize - offset);
            DEBUG('a', "Initializing code segment, at paddr 0x%x, size %d\n", 
                            paddr, _size);

            executable->ReadAt(&(machine->mainMemory[paddr]), _size, fileAddr);

            fileAddr += _size;
            offset = 0;
            page++;
        }
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing initData segment, at vaddr 0x%x, size %d\n", 
                        noffH.initData.virtualAddr, noffH.initData.size);

        // initialize the initData segment one page at a time
        int page = noffH.initData.virtualAddr / PageSize;
        int offset = noffH.initData.virtualAddr % PageSize; // only !=0 for page 1
        DEBUG('a', "Offset is %d\n", offset);
        int fileAddr = noffH.initData.inFileAddr;
        int _size = -1;
        for( int data = noffH.initData.size; data > 0; data -= _size) {
            DEBUG('a', "  %d bytes remain\n", data);
            ASSERT(pageTable[page].valid);
            int paddr = (pageTable[page].physicalPage * PageSize) + offset;
            _size = min(data, PageSize - offset);
            DEBUG('a', "Initializing initData segment, at paddr 0x%x, size %d\n", 
                            paddr, _size);

            executable->ReadAt(&(machine->mainMemory[paddr]), _size, fileAddr);

            fileAddr += _size;
            offset = 0;
            page++;
        }
    }
#endif                                         
}
#endif

// Load the Virtual Page at virtAddr to PageNumber in memory
/*
int AddrSpace::loadVirtualPage( unsigned int virtAddr, int PageNumber){
    
    if(!executable){
        DEBUG('a', "Invalid Executable\n");
    }
    
    if(pageTable == 0){
        // Construct the page table
        pageTable = new TranslationEntry[numPages];
    }

    InvertedPageTableEntry page;

    int virtPage = virtAddr / PageSize;
    
    // Copying from the Addrspace constructor
    NoffHeader noffH;
    unsigned int size, stackPageNumber, neededPages;

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);
    
        // read the header into noffH
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    
        // switch to big endian if it is not already
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC)){
        SwapHeader(&noffH);
    }
        // Check magic number
    ASSERT(noffH.noffMagic == NOFFMAGIC);
    
    // Calculate sizes and the pages required.
    dataSize = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    dataPages = divRoundUp(dataSize, PageSize);
    neededPages = dataPages + divRoundUp(UserStackSize,PageSize);
    
    DEBUG('a', "Initializing address space with %d valid pages, size %d\n",
                      neededPages, neededPages*PageSize);
        // and its stack sits in the last pages of the address space
    unsigned int stackStart = 
                    NumPhysPages - divRoundUp(UserStackSize, PageSize);
    numPages = neededPages;
    size = numPages * PageSize;
    
    // Read the executable into memory with all the segments and stuff
    // Max, you may want to do this. I couldn't understand the reading and
    // memory things.
    
    return PageNumber;
}
*/
//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifdef CHANGED
    // free the physical memory being used by this page table
#ifndef USE_TLB
    for(unsigned int i=0; i < numPages; i++) {
        physMemMapLock->Acquire();
        physMemMap.Clear(pageTable[i].physicalPage);
        physMemMapLock->Release();
    }
#endif
    delete pageTable;
        //delete childLock;
    delete stackTableLock;
    delete stackTable;

    // close any remaining files
    for(int i=0; i < MaxOpenFiles; i++) {
        OpenFile* f = (OpenFile*) fileTable.Remove(i);
        if(f) {
            delete f;
        }
    }
    
    // kill all child processes
    //set<PID>::iterator childItr;
    //while(childThread.size()>0){
    //    childItr = childThread.begin();
    //    this->removeChild((PID) *childItr);
    //}
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
#ifdef CHANGED
    unsigned int stackRegister = NumPhysPages * PageSize - 16;
    machine->WriteRegister(StackReg, stackRegister);
    DEBUG('a', "Initializing stack register to %x for stack 0\n", 
          stackRegister);
#endif
}

#ifdef CHANGED
//----------------------------------------------------------------------
// AddrSpace::InitStack
//      Create a new stack in the Addrspace and return it's stack id
//      Note that stack ids are local to this process
//----------------------------------------------------------------------

int AddrSpace::InitStack() {
    // find the first open stack in this process
    stackTableLock->Acquire();
    int stack = stackTable->Find();
    stackTableLock->Release();

    // check if there was room for another stack
    if(stack < 0) {
        // this needs to be handled appropriately
        cerr << "ERROR: Stack limit exceeded, no new stacks can be created!\n";
        return -1;
    }

    int stackPages = divRoundUp(UserStackSize,PageSize); //pages per stack

    // first page in this stack
    // [...data...][-unallocated-][stack][lower stacks...]
    int start = numPages - (stackPages * (stack + 1));

    // allocate the memory for this stack
    for(int i=start; i < stackPages + start; i++) {
        // find a free page in physical memory
        physMemMapLock->Acquire();
        int physPage = physMemMap.Find();
        ASSERT(physPage != -1); // make sure a page was found
        physMemMapLock->Release();

        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = physPage;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;

        // zero out the physical memory associated with this page
        bzero(&(machine->mainMemory[PageSize * physPage]), PageSize);
    }

    // Set the stack register to the end of the address space for this stack;
    // but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    unsigned int stackRegister = (start + stackPages) * PageSize - 16;
    machine->WriteRegister(StackReg, stackRegister);
    DEBUG('a', "Initializing stack register to %x for stack %d\n", 
          stackRegister, stack);

    return stack;
}

//----------------------------------------------------------------------
// AddrSpace::ClearStack
//      Remove the stack identified by the passed id
//----------------------------------------------------------------------

void AddrSpace::ClearStack(int id) {
    stackTableLock->Acquire();
    if(!stackTable->Test(id)) {
        // this stack is not in use
        cerr << "ERROR: tried to clear stack [" << id 
             << "] that was not allocated\n";
        return;
    }
    
    // pages per stack
    int stackPages = divRoundUp(UserStackSize,PageSize);

    // lowest index page of this stack
    int start = numPages - (stackPages * (id + 1));

    // free the physical memory associated with this stack
    for(int i = start; i < start + stackPages; i++) {
        physMemMapLock->Acquire();
        physMemMap.Clear(pageTable[i].physicalPage);
        physMemMapLock->Release();

        pageTable[i].physicalPage = 0;
        pageTable[i].valid = FALSE;
    }

    stackTable->Clear(id);
    stackTableLock->Release();

    return;
}
#endif

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
    // All the pages in the machine->TLB must be invalidated now
#ifdef USE_TLB
    for(int i=0; i < TLBSize; i++){
        machine->tlb[i].valid = FALSE;
    }
#endif
}

#ifdef CHANGED
//----------------------------------------------------------------------
// AddrSpace::readCString
//      Read a c-style string stored at the virtual address s
//      into a string object (so the kernel can use it)
//
//----------------------------------------------------------------------

std::string AddrSpace::readCString(char* s) {
    std::string ret = "";

    unsigned int page = (unsigned int) s / PageSize;
    if(page >= numPages || !pageTable[page].valid) {
        cerr << "ERROR: virtual address [" << (unsigned int) s 
             << "] passed to readCString is invalid\n"
             << " segmentation fault?\n";
        return ret;
    }
    unsigned int offset = (unsigned int) s % PageSize;
    unsigned int paddr = (pageTable[page].physicalPage * PageSize) + offset;

    // read the string till we hit null
    while(machine->mainMemory[paddr] != 0) {
        // append this char
        ret += machine->mainMemory[paddr];

        // update the physical address
        offset++;
        if(offset >= PageSize) {
            offset = 0;
            page++;
            if(page >= numPages || !pageTable[page].valid) {
                cerr << "ERROR: virtual address [" << (unsigned int) s 
                     << "] passed to readCString is invalid\n"
                     << " string prematurely truncated to " << ret << endl
                     << " segmentation fault?\n";
                return ret;
            }
        }
        paddr = (pageTable[page].physicalPage * PageSize) + offset;
    }

    return ret;
}
#endif

