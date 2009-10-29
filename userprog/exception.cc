    // exception.cc 
    //	Entry point into the Nachos kernel from user programs.
    //	There are two kinds of things that can cause control to
    //	transfer back to here from user code:
    //
    //	syscall -- The user code explicitly requests to call a procedure
    //	in the Nachos kernel.  Right now, the only function we support is
    //	"Halt".
    //
    //	exceptions -- The user code does something that the CPU can't handle.
    //	For instance, accessing memory that doesn't exist, arithmetic errors,
    //	etc.  
    //
    //	Interrupts (which can also cause control to transfer from user
    //	code into the Nachos kernel) are handled elsewhere.
    //
    // For now, this only handles the Halt() system call.
    // Everything else core dumps.
    //
    // Copyright (c) 1992-1993 The Regents of the University of California.
    // All rights reserved.  See copyright.h for copyright notice and limitation 
    // of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include "addrspace.h"
#include <cstdio>
#include <ctime>
#include <cstring>
#include <iostream>

#ifdef NETWORK
#define MaxDataSize (MaxMailSize - sizeof(unsigned) -sizeof(int))
int sequenceNumber = 0;
#endif

using namespace std;
extern "C" { int bzero(char *, int); };
Lock *processTableLock = new Lock("processTableLock");
Lock* locksTableLock = new Lock("locksTableLock");
Lock* CVTableLock = new Lock("CVTableLock");
static int TLBIndex = 0;
int curAge=0;
int faults=0;

struct ConditionWrapper {
    Condition* cv;
    int counter;
    bool mark;
    
    ConditionWrapper(Condition *c, int count, bool m){
        cv = c;
        counter = count;
        mark = m;
    }
};

struct LockWrapper {
    Lock* lock;
    int counter;
    bool mark;
    
    LockWrapper(Lock* l, int count, bool m){
        lock = l;
        counter = count;
        mark = m;
    }
};

int copyin(unsigned int vaddr, int len, char *buf) {
        // Copy len bytes from the current thread's virtual address vaddr.
        // Return the number of bytes so read, or -1 if an error occors.
        // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;
    
    while ( n >= 0 && n < len) {
        result = machine->ReadMem( vaddr, 1, paddr );
        while(!result) // FALL 09 CHANGES
        {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
        }	
        
        buf[n++] = *paddr;
        
        if ( !result ) {
                //translation failed
            return -1;
        }
        
        vaddr++;
    }
    
    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
        // Copy len bytes to the current thread's virtual address vaddr.
        // Return the number of bytes so written, or -1 if an error
        // occors.  Errors can generally mean a bad virtual address was
        // passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    
    while ( n >= 0 && n < len) {
            // Note that we check every byte's address
#ifndef USE_TLB
        result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );
        
        if ( !result ) {
                //translation failed
            return -1;
        }
#endif
#ifdef USE_TLB
        while(!machine->WriteMem( vaddr, 1, (int)(buf[n++]) ));
        n++;
#endif
        vaddr++;
    }
    
    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
        // Create the file with the name in the user buffer pointed to by
        // vaddr.  The file name is at most MAXFILENAME chars long.  No
        // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    
    if (!buf) return;
    
    if( copyin(vaddr,len,buf) == -1 ) {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }
    
    buf[len]='\0';
    
    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
        // Open the file with the name in the user buffer pointed to by
        // vaddr.  The file name is at most MAXFILENAME chars long.  If
        // the file is opened successfully, it is put in the address
        // space's file table and an id returned that can find the file
        // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id
    
    if (!buf) {
        printf("%s","Can't allocate kernel buffer in Open\n");
        return -1;
    }
    
    if( copyin(vaddr,len,buf) == -1 ) {
        printf("%s","Bad pointer passed to Open\n");
        delete[] buf;
        return -1;
    }
    
    buf[len]='\0';
    
    f = fileSystem->Open(buf);
    delete[] buf;
    
    if ( f ) {
        if ((id = currentThread->space->fileTable.Put(f)) == -1 )
            delete f;
        return id;
    }
    else
        return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
        // Write the buffer to the given disk file.  If ConsoleOutput is
        // the fileID, data goes to the synchronized console instead.  If
        // a Write arrives for the synchronized Console, and no such
        // console exists, create one. For disk files, the file is looked
        // up in the current address space's open file table and used as
        // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output
    
    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
        printf("%s","Error allocating kernel buffer for write!\n");
        return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
            printf("%s","Bad pointer passed to to write: data not written\n");
            delete[] buf;
            return;
        }
    }
    
    if ( id == ConsoleOutput) {
        for (int ii=0; ii<len; ii++) {
            printf("%c",buf[ii]);
        }
        
    } else {
        if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
            f->Write(buf, len);
        } else {
            printf("%s","Bad OpenFileId passed to Write\n");
            len = -1;
        }
    }
    
    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
        // Write the buffer to the given disk file.  If ConsoleOutput is
        // the fileID, data goes to the synchronized console instead.  If
        // a Write arrives for the synchronized Console, and no such
        // console exists, create one.    We reuse len as the number of bytes
        // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output
    
    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
        printf("%s","Error allocating kernel buffer in Read\n");
        return -1;
    }
    
    if ( id == ConsoleInput) {
            //Reading from the keyboard
        scanf("%s", buf);
        
        if ( copyout(vaddr, len, buf) == -1 ) {
            printf("%s","Bad pointer passed to Read: data not copied\n");
        }
    } else {
        if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
            len = f->Read(buf, len);
            if ( len > 0 ) {
                    //Read something from the file. Put into user's address space
                if ( copyout(vaddr, len, buf) == -1 ) {
                    printf("%s","Bad pointer passed to Read: data not copied\n");
                }
            }
        } else {
            printf("%s","Bad OpenFileId passed to Read\n");
            len = -1;
        }
    }
    
    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
        // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);
    
    if ( f ) {
        delete f;
    } else {
        printf("%s","Tried to close an unopen file\n");
    }
}

void kernel_thread(int virtAddr){
        // Setup new thread.
    machine->WriteRegister(PCReg, virtAddr);
    machine->WriteRegister(NextPCReg, virtAddr+4);
    currentThread->space->RestoreState();
        // Allocate Stack space for the new Thread and write the stackstart
        // address to the stack register.
    int stackId = currentThread->space->InitStack();
    if(stackId < 0){
        DEBUG('a', "%s: Unable to allocate stack for the new process\n",currentThread->getName());
            // Kill Process and all its children
            //    currentThread->space->killAllThreads();
        return;
    }
    machine->Run();
}

void Fork_Syscall(int funcAddr){
    processTableLock->Acquire();
    cout<<"Forking thread"<<endl;
    DEBUG('a', "%s: Called Fork_Syscall.\n",currentThread->getName());
        // Create new thread.kernel_thread()
    Thread *t = new Thread(currentThread->getName());
        // Stack was successfully created.
        // Add Process to the system's process table.
//    currentThread->space->childThreads++;
    int myPID = processTable->addProcess(currentThread->getPID()); 
        // Address space for new Thread and the spawning thread is the same.
    t->space = currentThread->space;
    t->setPID(myPID);
    t->Fork((VoidFunctionPtr) kernel_thread, funcAddr);
        // Restore state.
    currentThread->space->RestoreState();
    processTableLock->Release();
}

void exec_thread(int arg){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
    ASSERT(false);
        // We should never reach here.
}

spaceId Exec_Syscall(char *filename){
    processTableLock->Acquire();
    DEBUG('a', "%s: Called Exec_Syscall\n",currentThread->getName());
        // Get filename to kernel space
    std::string cname = currentThread->space->readCString(filename);
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());
    OpenFile *executable = fileSystem->Open(c_name);
    if(!executable){
        DEBUG('a',"%s: Unable to open file %s .\n", currentThread->getName(),c_name);
        cout<<"Exec_syscall: Unable to open file "<<c_name<<endl;
        return -1;
    }
        // Create new thread.
    Thread *t = new Thread(c_name);
        // Create new Address space and allocate it to the thread.
    t->space = new AddrSpace(executable);
        // Add process to process table.
    //processTable->processCounter++;
    int myPID = processTable->addProcess(currentThread->getPID()); 
    t->setPID(myPID);
    
    DEBUG('a', "%s: New thread created with PID: %d.\n",currentThread->getName(), t->getPID());
    processTableLock->Release();
#ifndef USE_TLB
    delete executable;
#endif
    t->Fork((VoidFunctionPtr) exec_thread, 0);
    return (spaceId) t->space;
}

void Exit_Syscall(int status){
    cout <<currentThread->getName()<<": Exit status: "<<status<<endl;
    processTableLock->Acquire();
    if (processTable->processCounter == 0 && currentThread->space->childThreads == 1) {
        DEBUG('a', "Exit_Syscall:End of NACHOS\n");
        interrupt->Halt();
    }else if (currentThread->space->childThreads == 1) {
        processTable->processCounter--;
        DEBUG('a', "Exit_Syscall:End of Process PID: %d\n", currentThread->getPID());
        processTableLock->Release();
        currentThread->Finish();
    }else {
            //Neither the end of process nor the end of Nachos
        DEBUG('a',"Exit_Syscall:End of Thread PID: %d\n", currentThread->getPID());
        currentThread->space->childThreads--;
        processTableLock->Release();
        currentThread->Finish();
    }
}


void Yield_Syscall(){
    (void) currentThread->Yield();
}

LockId CreateLock_Syscall(char* name){
    DEBUG('a',"%s: CreateLock_Syscall initiated.\n", currentThread->getName());
    locksTableLock->Acquire();
    std::string cname = currentThread->space->readCString(name);
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());
    
    LockWrapper *newLock = new LockWrapper(new Lock(c_name), 0, false);
        //    Lock *newLock = new Lock(c_name);    
    int retval;
    if (newLock) {
        if ((retval = currentThread->space->locksTable.Put(newLock)) == -1) {
                //unable to put the lock into the locktable
            delete newLock;
            retval = -1;
        }else {
                //All OK
        }
    }
    locksTableLock->Release();
    return retval;
}

void DestroyLock_Syscall(LockId id){
    locksTableLock->Acquire();
    LockWrapper *targetLock = (LockWrapper*) currentThread->space->locksTable.Get(id);
    if (targetLock) {
        if (targetLock->mark && targetLock->counter == 0) {
                //We can delete
            delete targetLock->lock;
            delete targetLock;
            DEBUG('a',"%s: DestroyLock_Syscall: Successfully deleted lock %d .\n",
                  currentThread->getName(), id);
        }else {
                //Cannot delete need to persist just mark for deletion
            targetLock->mark = true;
            DEBUG('a',"%s: DestroyLock_Syscall: marked lock %d for deletion.\n",
                  currentThread->getName(), id);
        }
        
    }else {
            //Unable to find targetLock
        DEBUG('a',"%s: DestroyLock_Syscall: Unable to find lock %d for deletion.\n",
              currentThread->getName(), id);
    }
    currentThread->space->locksTable.Remove(id);
    locksTableLock->Release();
}

void AcquireLock_Syscall(LockId lockId){
    locksTableLock->Acquire();
    LockWrapper *targetLock = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    if(targetLock == NULL){
        DEBUG('a',"%s: AcquireLock_Syscall: Unable to find lock %d for acquire.\n",
              currentThread->getName(), lockId);
    }else{
        DEBUG('a',"%s: Lock %d: AcquireLock_Syscall.\n",currentThread->getName(),lockId);
        targetLock->counter++;
        locksTableLock->Release();
        
        targetLock->lock->Acquire();
        locksTableLock->Acquire();
        targetLock->counter--;
    }
    locksTableLock->Release();
}

void ReleaseLock_Syscall(LockId lockId){
    locksTableLock->Acquire();
    LockWrapper *targetLock = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    if(targetLock == NULL){
        DEBUG('a',"%s: AcquireLock_Syscall: Unable to find lock %d for acquire.\n",
              currentThread->getName(), lockId);
    }else{
        DEBUG('a',"%s: Lock %d: ReleaseLock_Syscall.\n",currentThread->getName(),lockId);
        targetLock->lock->Release();
    }
    locksTableLock->Release();
}


CVId CreateCondition_Syscall(char* name){
    DEBUG('a',"%s : CreateCondition_Syscall initialized.\n",currentThread->getName());
    std::string cname = currentThread->space->readCString(name);
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());    
    Condition *newcon = new Condition(c_name);
    if(newcon == NULL){
        return -1;
    }
    
	ConditionWrapper *newCV = new ConditionWrapper(newcon, 0, false);
	
    int retval;
	if(newCV){
        CVTableLock->Acquire();
		if((retval = currentThread->space->CVTable.Put(newCV)) == -1){
			delete newCV;
		}else{
            CVTableLock->Release();
                //printf("CREA\n");
			return retval;
		}
	}
    CVTableLock->Release();
	return -1;
}

void DestroyCondition_Syscall(CVId id){
    DEBUG('a', "DestroyCondition syscall.\n");
    CVTableLock->Acquire();
	ConditionWrapper *target = (ConditionWrapper*) currentThread->space->CVTable.Get(id);
    if(target){
        if (target->counter == 0) {
                //We can delete
            delete target->cv;
            delete target;
            DEBUG('a',"%s : DestroyCondition_Syscall: Successfully deleted CV %d .\n",
                  currentThread->getName(), id);
            currentThread->space->CVTable.Remove(id);
        }else {
                //Cannot delete need to persist just mark for deletion
            target->mark = true;
            DEBUG('a',"%s: DestroyCondition_Syscall: marked CV %d for deletion.\n",
                  currentThread->getName(), id);
        }        
    }else{
        DEBUG('a',"%s: DestroyCondition_Syscall: Unable to find CV %d for deletion.\n",
              currentThread->getName(), id);
    }
    CVTableLock->Release();
}

void WaitCV_Syscall(CVId cvId, LockId lockId){
    CVTableLock->Acquire();
    locksTableLock->Acquire();
    
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    if (ConditionLockWrapper == NULL || CV == NULL || ConditionLockWrapper->lock == NULL || 
        CV->cv == NULL) {
        CVTableLock->Release();
        locksTableLock->Release();
        DEBUG('a',"%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
              currentThread->getName(), cvId, lockId);
        printf("%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
               currentThread->getName(), cvId, lockId);
        return;
    }
        //Set wait on this condition variable
    DEBUG('a',"%s: WaitCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    CV->counter++;
    locksTableLock->Release();
    CVTableLock->Release();
    (void) CV->cv->Wait(ConditionLockWrapper->lock);
    
    CVTableLock->Acquire();
    printf("current counter: %d",CV->counter--);
    CVTableLock->Release();
}

void SignalCV_Syscall(CVId cvId, LockId lockId){
    CVTableLock->Acquire();
    locksTableLock->Acquire();
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    
    if (ConditionLockWrapper == NULL || 
        CV == NULL || 
        ConditionLockWrapper->lock == NULL || 
        CV->cv == NULL) {
        CVTableLock->Release();
        locksTableLock->Release();
        DEBUG('a',"%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
              currentThread->getName(), cvId, lockId);
        printf("%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
               currentThread->getName(), cvId, lockId);
        return;
    }
    
        //Set wait on this condition variable
    DEBUG('a',"%s: SignalCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    (void) CV->cv->Signal(ConditionLockWrapper->lock);
    CVTableLock->Release();
    locksTableLock->Release();
}

void BroadcastCV_Syscall(CVId cvId, LockId lockId){
    CVTableLock->Acquire();
    locksTableLock->Acquire();
    
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    
    if (ConditionLockWrapper == NULL || 
        CV == NULL || 
        ConditionLockWrapper->lock == NULL || 
        CV->cv == NULL) {
        DEBUG('a',"%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
              currentThread->getName(), cvId, lockId);
        printf("%s: WaitCV_Syscall: Failed for CVId: %d lockId %d .\n", 
               currentThread->getName(), cvId, lockId);
        CVTableLock->Release();
        locksTableLock->Release();
        return;
    }
    
        //Set wait on this condition variable
    DEBUG('a',"%s: BroadcastCV_Syscall: Called for CVId: %d lockId %d .\n",
          currentThread->getName(), cvId, lockId);
    (void) CV->cv->Broadcast(ConditionLockWrapper->lock);
    CVTableLock->Release();
    locksTableLock->Release();
}

int Random_Syscall(){
    return Random();
}

int getTimestamp(){
    return (int) time(0);
}
#ifdef USE_TLB

void CopyTranslationEntry(TranslationEntry* sourceTE,TranslationEntry* destTE){
    // Perform deep copy the source to the destination
    
    destTE->virtualPage = sourceTE->virtualPage;
    destTE->physicalPage = sourceTE->physicalPage;
    destTE->valid = sourceTE->valid;
    destTE->use = sourceTE->use;
    destTE->dirty = sourceTE->dirty;
    destTE->readOnly = sourceTE->readOnly;
}

void CopyTLB2IPT(){
    for (int i=0; i<TLBSize; i++)
        if(machine->tlb[i].valid)
            CopyTranslationEntry(&(machine->tlb[i]),&(IPT[machine->tlb[i].physicalPage]));
}

int findInIPT(int vAddr,int PID){
    for(int i=0;i<NumPhysPages;i++)
        if(IPT[i].virtualPage == vAddr && IPT[i].PID == PID)
            return i;
    // Didn't find the page in IPT!!
    return -1;
}
int SelectPageToBeSwapped()
{
    static int physicalPage = -1;
    // All pages are full - time to swap pages
    if(!FIFOreplacementPolicy){
        physicalPage = Random() % NumPhysPages;
        DEBUG('a', "Random selection swaping: %d.\n",physicalPage);
    }else{
        DEBUG('a', "FIFO selection.\n");
        for(int i=0; i<NumPhysPages; i++){
            if(IPT[i].age < IPT[physicalPage].age){
                physicalPage = i;
            }
        }
    }    
    return physicalPage;
}

int findAvailablePage(){
    int virtualPage = -1, physicalPage = -1;
    AddrSpace *targetSpace;
    // Find a free page in the IPT, if any
    for(int i=0;i<NumPhysPages;i++){
        if(IPT[i].physicalPage == -1){
            // This page is free!!!
            DEBUG('a', "Found a free page at %d\n", i);
            physicalPage = i;
            // Yippie!!
            return physicalPage;
        }
    }
    // Need to swap pages!!
    physicalPage = SelectPageToBeSwapped();
    virtualPage = IPT[physicalPage].virtualPage;
    targetSpace = IPT[physicalPage].space;
    
    if(IPT[physicalPage].dirty){
        // Page modified but not committed.
        if(IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation == -1){
            IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation = swapLocation++;
        }
        // Write the page to swapLocation.
        swapFile->WriteAt(&(machine->mainMemory[PageSize * physicalPage]),PageSize,
                          (PageSize * IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation));
        IPT[physicalPage].space->PageTableInfo[virtualPage].PageStatus = SWAP;
    }else if(IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation == -1){
        // Okay!! page is not dirty and was never written to swap file...
        IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation = swapLocation++;
        swapFile->WriteAt(&(machine->mainMemory[PageSize * physicalPage]),PageSize,
                          (PageSize * IPT[physicalPage].space->PageTableInfo[virtualPage].swapLocation));
        IPT[physicalPage].space->PageTableInfo[virtualPage].PageStatus = SWAP;
    }else {
        IPT[physicalPage].space->PageTableInfo[virtualPage].PageStatus = SWAP;
    }
    
    //If currentThread, invalidate all entries in TLB
    if(IPT[physicalPage].space == currentThread->space){
        for(int i=0; i< TLBSize; i++){
            if(machine->tlb[i].virtualPage == virtualPage){
                machine->tlb[i].valid = false;
            }
        }
    }
    return physicalPage;
}

void handlePageFaultException(int vAddr){
    cout << "handlePageFaultException: vAddr= "<<vAddr<<endl;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int virtualpage = vAddr / PageSize;
    int physicalPage = -1;
    int tlbpos = 0;
    DEBUG('a', "Handling PageFault for VADDR: %d\n", vAddr);
    tlbpos = TLBIndex;
    TLBIndex = (TLBIndex+1) % TLBSize;
    DEBUG('a', "TLBIndex: %d tlbpos: %d\n",TLBIndex, tlbpos);
    // Copy out all pages from the TLB - update the IPT
    CopyTLB2IPT();
    // Now we check if the currentThread->space pageTable is in memory
    // If yes, load from IPT
    cout << "currentThread->space->PageTableInfo[virtualpage].PageStatus = "
         << currentThread->space->PageTableInfo[virtualpage].PageStatus
         <<endl;
    
    if(currentThread->space->PageTableInfo[virtualpage].PageStatus == MEMORY){
        // Find page in IPT
        physicalPage = findInIPT(virtualpage, currentThread->PID);
        // make sure the page was actually found where it is supposed to be
        ASSERT(physicalPage != -1);
        DEBUG('a',"Working with Physical Page: %d\n",physicalPage);
        // Copy IPT -> TLB
        CopyTranslationEntry( &(IPT[physicalPage]), &(machine->tlb[tlbpos]) );
        (void) interrupt->SetLevel(oldLevel);
        return;
    } // END OF MEMORY MATCH

    // Find a free page in memory to get th page in - FindOpenPhysicalPage
    physicalPage = findAvailablePage();
    DEBUG('a',"physicalPage Value: %d\n",physicalPage);
    // So by now we have the physical page which we want to screw with.
    // so, zero it out!!
    bzero(&(machine->mainMemory[PageSize * physicalPage]), PageSize);

    if(currentThread->space->PageTableInfo[virtualpage].PageStatus == EXEC){
        // Load from file!
        DEBUG('a', "Loading page from file: %d, virtualPage: %d\n", 
              currentThread->space->noffH.code.inFileAddr, 
              virtualpage);
        currentThread->space->executable->ReadAt(
                            &(machine->mainMemory[physicalPage * PageSize]), // location in memory (target)
                            PageSize, // size
                            currentThread->space->noffH.code.inFileAddr + virtualpage * PageSize // length
                            );
    }else if(currentThread->space->PageTableInfo[virtualpage].PageStatus == SWAP){
        DEBUG('a',"Load SwapFile\n");
        swapFile->ReadAt(
                         &(machine->mainMemory[physicalPage * PageSize]), 
                         PageSize,
                         (PageSize * currentThread->space->PageTableInfo[virtualpage].swapLocation));
    }else if (currentThread->space->PageTableInfo[virtualpage].PageStatus == UNINITDATA) {
        DEBUG('a',"Uninitialized Data\n");
    }
    DEBUG('a', "Page loading complete\n");
    // The page is now in memory - mark this state change!
    currentThread->space->PageTableInfo[virtualpage].PageStatus = MEMORY;
    // Now setup the IPT page with these values.
    IPT[physicalPage].virtualPage = virtualpage;
    IPT[physicalPage].physicalPage = physicalPage;
    IPT[physicalPage].valid = true;
    IPT[physicalPage].use = false;
    IPT[physicalPage].dirty = false;
    IPT[physicalPage].readOnly = false;
    IPT[physicalPage].PID = currentThread->PID;
    IPT[physicalPage].PageStatus = currentThread->space->PageTableInfo[virtualpage].PageStatus;
    IPT[physicalPage].swapLocation = currentThread->space->PageTableInfo[virtualpage].swapLocation;
    IPT[physicalPage].space = currentThread->space;
    
    // Now copy the IPT[physicalPage] to TLB[TLBIndex]
    CopyTranslationEntry(&(IPT[physicalPage]),&(machine->tlb[tlbpos]));
    // Everything is done!!
    (void) interrupt->SetLevel(oldLevel);
    return;
}
#endif
/* 
 * Receives the  message sent to mbox. 
 * Returns the number of bytes received
 */
int Receive_Syscall(int senderID, int mbox,int vaddr){
#ifdef NETWORK
    DEBUG('a', "Receive_Syscall\n");
    char *message = new char[MaxMailSize];
    bzero(message, MaxMailSize);
    
    PacketHeader pktHead;
    MailHeader mailHead;
    
    postOffice->Receive(senderID, &pktHead, &mailHead, message);
    // Copy message to vaddr
    return copyout(vaddr, sizeof(message), message);
#endif
}

void Send_Syscall(int receiverID,int mbox,int vaddr){
#ifdef NETWORK
    DEBUG('a', "Send_syscall initialized\n");
    char *message = new char[MaxMailSize];
    bzero(message, MaxMailSize);
    
    int senderID = netname;
    // Message Format[sequenceNumber, senderId, DATA]
    // get and increment sequence number
    unsigned curSequenceNumber = sequenceNumber++;
    // Add sequenceNo to the message
    memcpy( message, &curSequenceNumber, sizeof(unsigned) );
    // Add sender Id to message
    memcpy( message+sizeof(unsigned), &senderID, sizeof(int) );
    
    char *payload = new char[MaxDataSize];
    // read the payload to be sent
    int bytesRead = copyin(vaddr, MaxDataSize, payload);
    
    memcpy(message+sizeof(unsigned)+sizeof(int), payload, sizeof(payload) );
    cout << "MESSAGE: "<<message;
    if (bytesRead != -1) {
        // Payload successfully acquired
        DEBUG('a', "Payload: %s, Receiver: %d, Receiver Mailbox: %d\n", payload, receiverID, mbox);
        
        // Send packet
        PacketHeader pktHead;
        pktHead.to = receiverID;
        
        MailHeader mailHead;
        mailHead.to = mbox;
        mailHead.from = senderID;
        mailHead.length = strlen(message) + 1;
        // Send the message to other client
        bool retVal = postOffice->Send(pktHead, mailHead, message);
        if (!retVal) {
            DEBUG('a', "Cannot Send\n");
            interrupt->Halt();
        }else {
            DEBUG('a', "Message sent: %s\n", message);
        }
    }else {
        DEBUG('a', "Failed to read Payload\n");
    }
#endif
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall
    
    if ( which == SyscallException ) {
        switch (type) {
            default:
                DEBUG('a', "Unknown syscall - shutting down.\n");
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Create:
                DEBUG('a', "Create syscall.\n");
                Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Open:
                DEBUG('a', "Open syscall.\n");
                rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Write:
                DEBUG('a', "Write syscall.\n");
                Write_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5),
                              machine->ReadRegister(6));
                break;
            case SC_Read:
                DEBUG('a', "Read syscall.\n");
                rv = Read_Syscall(machine->ReadRegister(4),
                                  machine->ReadRegister(5),
                                  machine->ReadRegister(6));
                break;
            case SC_Close:
                DEBUG('a', "Close syscall.\n");
                Close_Syscall(machine->ReadRegister(4));
                break;
#ifdef CHANGED
            case SC_Fork:
                DEBUG('a', "Fork syscall.\n");
                Fork_Syscall(machine->ReadRegister(4));
                break;
            case SC_Exec:
                DEBUG('a', "Exec syscall.\n");
                rv = Exec_Syscall((char*) machine->ReadRegister(4));
                break;
            case SC_Exit:
                DEBUG('a', "Exit syscall.\n");
                Exit_Syscall((int) machine->ReadRegister(4));
                break;
            case SC_Yield:
                (void) Yield_Syscall();
                break;                
            case SC_CreateLock:
                DEBUG('a', "CreateLock syscall.\n");
                rv = CreateLock_Syscall((char*) machine->ReadRegister(4));
                break;
            case SC_DestroyLock:
                DestroyLock_Syscall((LockId) machine->ReadRegister(4));
                break;
            case SC_Acquire:
                AcquireLock_Syscall((LockId) machine->ReadRegister(4));
                break;
            case SC_Release:
                ReleaseLock_Syscall((LockId) machine->ReadRegister(4));
                break;
            case SC_CreateCondition:
                rv = CreateCondition_Syscall((char*) machine->ReadRegister(4));
                break;
            case SC_DestroyCondition:
            	DestroyCondition_Syscall((CVId) machine->ReadRegister(4));
                break;
            case SC_Wait:
                WaitCV_Syscall((CVId) machine->ReadRegister(4),
                               (LockId) machine->ReadRegister(5));
                break;
            case SC_Signal:
                SignalCV_Syscall((CVId) machine->ReadRegister(4),
                                 (LockId) machine->ReadRegister(5));
                break;
            case SC_Broadcast:
                BroadcastCV_Syscall((CVId) machine->ReadRegister(4),
                                    (LockId) machine->ReadRegister(5));
                break;
            case SC_Random:
                DEBUG('a', "Random syscall.\n");
                rv = Random_Syscall();
                break;
            case SC_Send:
                DEBUG('a', "Send_Syscal\n");
                Send_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),
                             machine->ReadRegister(6));
                break;
            case SC_Receive:
                DEBUG('a', "Receive_Syscall\n");
                rv =  Receive_Syscall(machine->ReadRegister(4), 
                                      machine->ReadRegister(5),
                                      machine->ReadRegister(6));
                break;
#endif
        }
        
            // Put in the return value and increment the PC
        machine->WriteRegister(2,rv);
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
        return;
    }
#ifdef USE_TLB
    else if(which == PageFaultException){
        DEBUG('s',"Caught PageFaultException!\n");
        handlePageFaultException(machine->ReadRegister(BadVAddrReg));
    }
#endif 
    else {
        cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
        interrupt->Halt();
    }
}
