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
#include <sstream>
#include <vector>
#ifdef CHANGED
#include <algorithm>
#endif

#ifdef NETWORK
int getTimestamp(){
    time_t ltime;
    ltime = time(NULL);
    return ltime;
}
#endif

using namespace std;
#ifdef NETWORK
class Packet {
    public:
    int senderId;
    int timestamp;
    char data[MaxMailSize - 2 * sizeof(int)];

    // message MUST be a pointer to a char array of MaxMailSize
    char* Serialize(char *message){
        message[1] = senderId;
        mesage[0] = senderId >> 8;
        message[3] = timestamp;
        message[2] = timestamp >> 8;
        for(unsigned i = 4; i< MaxMailSize; i++)
            message[i] = data[i-4];
        return message;
    }

    void Deserialize(char *message){
        senderId = (int)((message[0] * 256) + message[1]);
        timestamp = (int)((message[2] * 256) + message[3]);
        for(unsigned i=4; i< MaxMailSize; i++)
            data[i-4] = message[i];
    }
}; 
#endif
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
    
    while ( n >= 0 && n <= len) {
            // Note that we check every byte's address
        result = machine->WriteMem( vaddr, 1, (buf[n]));
        while(!result){
            result = machine->WriteMem(vaddr, 1, buf[n]);
        }
        n++;
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
    //cerr << "kernel_thread called for virtAddr: " << virtAddr << endl;
    machine->WriteRegister(PCReg, virtAddr);
    machine->WriteRegister(NextPCReg, virtAddr+4);
    currentThread->space->RestoreState();
        // Allocate Stack space for the new Thread and write the stackstart
        // address to the stack register.
    int stackId = currentThread->space->InitStack();
    //printf("StackId: %d\n", stackId);
    currentThread->stackId = stackId;
    if(stackId < 0){
        printf("%s: Unable to allocate stack for the new process\n",currentThread->getName());
        return;
    }
    machine->Run();
}

void Fork_Syscall(int funcAddr){
    printf("Fork Started\n");
    processTableLock->Acquire();
    DEBUG('a', "%s: Called Fork_Syscall.\n",currentThread->getName());
        // Create new thread.kernel_thread()
    Thread *t = new Thread("forked Thread");
        // Stack was successfully created.
        // Add Process to the system's process table.
    currentThread->space->childThreads++;
    int myPID = processTable->addProcess(currentThread->getPID()); 
        // Address space for new Thread and the spawning thread is the same.
    t->space = currentThread->space;
    t->setPID(myPID);
    t->Fork((VoidFunctionPtr) kernel_thread, funcAddr);
        // Restore state.
    currentThread->space->RestoreState();
    processTableLock->Release();
    printf("Fork Complete\n");
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
        processTableLock->Release();
        return -1;
    }

    // Create new thread.
    Thread *t = new Thread(c_name);
    
    // Create new Address space and allocate it to the thread.
    t->space = new AddrSpace(executable);
    // Add process to process table.
    //processTable->processCounter++;
    processTableLock->Release();
    int myPID = processTable->addProcess(currentThread->PID); 
    t->PID = myPID;
    t->space->PID = myPID; 
    DEBUG('a', "%s: New thread created with PID: %d.\n",currentThread->getName(), t->getPID());
    t->Fork((VoidFunctionPtr) exec_thread, 0);
    return (spaceId) t->space;
}

void Exit_Syscall(int status){
    cout<<"Exit Status: "<<status<<endl;
    processTableLock->Acquire();
    if (processTable->processCounter == 1 && currentThread->space->childThreads == 0) {
        printf("Exit_Syscall: End of NACHOS\n");
        interrupt->Halt();
    }else if (currentThread->space->childThreads == 0) {
        processTable->removeProcess(currentThread->PID);
        printf( "Exit_Syscall: End of Process PID: %d\n", currentThread->getPID());
        currentThread->space->ClearStack(currentThread->stackId);
        processTableLock->Release();
        currentThread->Finish();
    }else {
        //Neither the end of process nor the end of Nachos
        printf("Exit_Syscall: End of Thread PID: %d\n", currentThread->getPID());
        processTable->removeProcess(currentThread->PID);
        currentThread->space->childThreads--;
        //currentThread->space->ClearStack(currentThread->stackId);
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
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    for (int i=0; i<TLBSize; i++)
        if(machine->tlb[i].valid)
            CopyTranslationEntry(&(machine->tlb[i]),&(IPT[machine->tlb[i].physicalPage]));
    (void) interrupt->SetLevel(oldLevel);
}

int findInIPT(int vAddr,int PID){
    for(int i=0;i<NumPhysPages;i++)
        if(IPT[i].virtualPage == vAddr && IPT[i].valid && IPT[i].PID == PID)
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
        //printf( "FIFO selection.\n");
        physicalPage = (physicalPage + 1) % NumPhysPages;
        //cout<<"FIFO selected: "<<physicalPage<<endl;
    }    
    return physicalPage;
}

int findAvailablePage(){
    int vpn = -1, ppn = -1;
    AddrSpace *targetSpace;
    // Find a free page in the IPT, if any
    for(int i=0; i < NumPhysPages; i++){
        if(IPT[i].valid == false){
            // This page is free!!!
            ppn = i;
            // Yippie!!
            return ppn;
        }
    }
    // Didn't get a pagethat was free 
    // Need to swap pages!!
    ppn = SelectPageToBeSwapped();
    vpn = IPT[ppn].virtualPage;
    targetSpace = IPT[ppn].space;
    
    if(IPT[ppn].dirty){
        // Page modified but not committed.
        if(IPT[ppn].space->pageTableInfo[vpn].swapLocation == -1){
            // need to select a swap location
            swapLock->Acquire();
            int newSwapLocation = swapBitMap->Find();
            swapLock->Release();
            if(newSwapLocation == -1) {
                // Ran out of SWAP !!!
                cerr << "ERROR: Swap file FULL !!\n";
                Exit_Syscall(1);
                return -1;
            }
            IPT[ppn].space->pageTableInfo[vpn].swapLocation = newSwapLocation;
        }
        // Write the page to swapLocation.
        swapFile->WriteAt(&(machine->mainMemory[PageSize * ppn]),PageSize,
                          (PageSize * IPT[ppn].space->pageTableInfo[vpn].swapLocation));
        IPT[ppn].space->pageTableInfo[vpn].PageStatus = SWAP;
    }else if(IPT[ppn].space->pageTableInfo[vpn].swapLocation == -1){
        // Okay!! page is not dirty and was never written to swap file...
        // need to select a new swap location
        swapLock->Acquire();
        int newSwapLocation = swapBitMap->Find();
        swapLock->Release();
        if(newSwapLocation == -1) {
            // Ran out of SWAP !!!
            cerr << "ERROR: Swap file FULL !!\n";
            Exit_Syscall(1);
            return -1;
        }
        IPT[ppn].space->pageTableInfo[vpn].swapLocation = newSwapLocation;
        swapFile->WriteAt(&(machine->mainMemory[PageSize * ppn]),PageSize,
                          (PageSize * IPT[ppn].space->pageTableInfo[vpn].swapLocation));
        IPT[ppn].space->pageTableInfo[vpn].PageStatus = SWAP;
    }else {
        IPT[ppn].space->pageTableInfo[vpn].PageStatus = SWAP;
    }
    
    //If currentThread, invalidate all entries in TLB
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(IPT[ppn].space == currentThread->space){
        for(int i=0; i< TLBSize; i++){
            if(machine->tlb[i].virtualPage == vpn){
                machine->tlb[i].valid = false;
            }
        }
    }
    (void) interrupt->SetLevel(oldLevel);

    return ppn;
}

void loadPageFromExec(int ppn, int vpn) {
    int pageStart = vpn * PageSize;
    int pageEnd = pageStart + PageSize;
    int codeStart = currentThread->space->noffH.code.virtualAddr;
    int codeEnd = codeStart + currentThread->space->noffH.code.size;
    int initStart = currentThread->space->noffH.initData.virtualAddr;
    int initEnd = initStart + currentThread->space->noffH.initData.size;

    // There are 6 potential alignments between the code section and this page
    //  must make sure they are all handled
    bool read = true;
    int codeOffset = 0;
    int pageOffset = 0;
    int length = 0;
    if(codeStart < pageStart) {
        codeOffset = pageStart - codeStart;
        pageOffset = 0;
        if(codeEnd <= pageStart) {
            // do nothing
            read = false;
        } else {
            if(codeEnd >= pageEnd) {
                // copy into the whole page
                length = PageSize;
            } else {
                // copy the end of the code section to this page
                length = codeEnd - pageStart;
            }
        }
    } else {  //codeStart >= pageStart
        codeOffset = 0;
        pageOffset = codeStart - pageStart;
        if(codeStart >= pageEnd) {
            // do nothing
            read = false;
        } else {
            if(codeEnd >= pageEnd) {
                // copy start of code section to this page
                length = pageEnd - codeStart;
            } else {
                // copy whole code section to this page
                length = codeEnd - codeStart;
            }
        }
    }

    if(read) {
        //cout << "Loading page " << vpn << " from exec, codeOffset: " << codeOffset
        //     << " pageOffset: " << pageOffset << " length: " << length << endl;
        currentThread->space->executable->ReadAt(
            &(machine->mainMemory[(ppn * PageSize) + pageOffset]), // location in memory (target)
            length, // size
            currentThread->space->noffH.code.inFileAddr + codeOffset); // file location
    }

    //*** now do the initdata section
    read = true;
    int initOffset = 0;
    if(initStart < pageStart) {
        initOffset = pageStart - initStart;
        pageOffset = 0;
        if(initEnd <= pageStart) {
            // do nothing
            read = false;
        } else {
            if(initEnd >= pageEnd) {
                // copy into the whole page
                length = PageSize;
            } else {
                // copy the end of the init section to this page
                length = initEnd - pageStart;
            }
        }
    } else {  //initStart >= pageStart
        initOffset = 0;
        pageOffset = initStart - pageStart;
        if(initStart >= pageEnd) {
            // do nothing
            read = false;
        } else {
            if(initEnd >= pageEnd) {
                // copy start of init section to this page
                length = pageEnd - initStart;
            } else {
                // copy whole init section to this page
                length = initEnd - initStart;
            }
        }
    }
    
    if(read) {
        //cout << "Loading page " << vpn << " from exec, initStart: " << initStart << " initOffset: " << initOffset
        //     << " pageOffset: " << pageOffset << " length: " << length << endl;
        currentThread->space->executable->ReadAt(
            &(machine->mainMemory[(ppn * PageSize) + pageOffset]), // location in memory (target)
            length, // size
            currentThread->space->noffH.initData.inFileAddr + initOffset); // file location
    }

}

void handlePageFaultException(int vAddr){
    DEBUG('a', "Handling PageFault for VADDR: %d\n", vAddr);
    IntStatus oldLevel;
    oldLevel = interrupt->SetLevel(IntOff);
    int virtualpage = vAddr / PageSize;
    int physicalPage = -1;
    int tlbpos = 0;

    TLBIndexLock->Acquire();
    tlbpos = TLBIndex;
    TLBIndex = (TLBIndex+1) % TLBSize;
    DEBUG('a', "TLBIndex: %d tlbpos: %d\n",TLBIndex, tlbpos);
    TLBIndexLock->Release();

    // Copy out all pages from the TLB - update the IPT
    CopyTLB2IPT();

    // Now we check if the page is in memory
    // If yes, load from IPT
    //IPTLock->Acquire();
    physicalPage = findInIPT(virtualpage, currentThread->space->PID);

    if(physicalPage != -1) {
        // make sure the page was actually found where it is supposed to be
        DEBUG('a',"Working with Physical Page: %d\n",physicalPage);
        // Copy IPT -> TLB
        CopyTLB2IPT();
        CopyTranslationEntry( &(IPT[physicalPage]), &(machine->tlb[tlbpos]) );

        // RESTORE INTERRUPTS
        (void) interrupt->SetLevel(oldLevel);
        //IPTLock->Release();
        return;
    } // END OF MEMORY MATCH
    //IPTLock->Release();

    // must check if this page is valid first, kill currentThread with
    //  a segfault if it is not
    if(currentThread->space->pageTableInfo[virtualpage].valid == false) {
        // RESTORE INTERRUPTS
        cout << "ERROR: Virtual Page " << virtualpage << " is not valid\n"
             << " SEGFAULT!\n";
        // die die die
        (void) interrupt->SetLevel(oldLevel);
        Exit_Syscall(1);
        return;
    }

    //IPTLock->Acquire();
    // Find a free page in memory to get th page in - FindOpenPhysicalPage
    physicalPage = findAvailablePage();
    DEBUG('a',"physicalPage Value: %d\n",physicalPage);
    if(physicalPage == -1) {
        // RESTORE INTERRUPTS
        (void) interrupt->SetLevel(oldLevel);
        //IPTLock->Release();
        return;
    }

    // So by now we have the physical page which we want to screw with.
    // so, zero it out!!
    bzero(&(machine->mainMemory[PageSize * physicalPage]), PageSize);

    if(currentThread->space->pageTableInfo[virtualpage].PageStatus == EXEC){
        // Load from file!
        DEBUG('a', "Loading page from file: %d, virtualPage: %d\n", 
              currentThread->space->noffH.code.inFileAddr, 
              virtualpage);
        // load this page from the executable
        loadPageFromExec(physicalPage, virtualpage);
        
    }else if(currentThread->space->pageTableInfo[virtualpage].PageStatus == SWAP){
        DEBUG('a',"Load SwapFile\n");
        swapFile->ReadAt(
                         &(machine->mainMemory[physicalPage * PageSize]), 
                         PageSize,
                         (PageSize * currentThread->space->pageTableInfo[virtualpage].swapLocation));
    }else if (currentThread->space->pageTableInfo[virtualpage].PageStatus == NOWHERE) {
        DEBUG('a',"Page has not been allocated yet\n");
    }

    DEBUG('a', "Page loading complete\n");
    // The page is now in memory - mark this state change!
    currentThread->space->pageTableInfo[virtualpage].PageStatus = MEMORY;

    // Now setup the IPT page with these values.
    IPT[physicalPage].virtualPage = virtualpage;
    IPT[physicalPage].physicalPage = physicalPage;
    IPT[physicalPage].valid = true;
    IPT[physicalPage].use = false;
    IPT[physicalPage].dirty = false;
    IPT[physicalPage].readOnly = false;
    IPT[physicalPage].PID = currentThread->space->PID;
    IPT[physicalPage].PageStatus = currentThread->space->pageTableInfo[virtualpage].PageStatus;
    IPT[physicalPage].swapLocation = currentThread->space->pageTableInfo[virtualpage].swapLocation;
    IPT[physicalPage].space = currentThread->space;
    
    // Now copy the IPT[physicalPage] to TLB[TLBIndex]
    CopyTranslationEntry(&(IPT[physicalPage]),&(machine->tlb[tlbpos]));

    // RESTORE INTERRUPTS
    (void) interrupt->SetLevel(oldLevel);

    //IPTLock->Release();

    // Everything is done!!
    return;
}
#endif

/* 
 * Receives the  message sent to mbox. 
 * Returns the number of bytes received
 */
int Receive_Syscall(int senderID, int mbox, int vaddr){
#ifdef NETWORK
    int bytesRead = -1;
    char *message = new char[MaxMailSize];
    //cout<<"Inside Receive_SYSCALL\n"<<endl;
    bzero(message, MaxMailSize);
    
    PacketHeader pktHead;
    MailHeader mailHead;
    if (senderID >= 0 && mbox >=0) {
         postOffice->Receive(senderID, &pktHead, &mailHead, message);
    }else{
        interrupt->Halt();
    }
    // Copy message to vaddr

    // @ankur: Let the handling of the packet details happen in the userprogram
    //Packet pkt;
    //pkt.Deserialize(message);

    //cout << "Data received: "<<pkt.data<<endl;
    fflush(stdout);
    bytesRead = copyout(vaddr, sizeof(message), message);
    return bytesRead;
#endif

}

void Send_Syscall(int receiverID,int mbox,int vaddr){
#ifdef NETWORK
    Packet pkt;
    pkt.senderId = netname;
    pkt.timestamp = getTimestamp();
    
    char *payload = new char[32];
    int bytesRead = copyin(vaddr, 32 , pkt.data);
    
    //strcpy(pkt.data, payload);
    //cout<< "Sending: "<<payload<<endl;
    // Serialize everything to be sent
   char *message = new char[MaxMailSize];
   message = pkt.Serialize(message);
    if (bytesRead != -1) {
        // Payload successfully acquired
        // Send packet
        PacketHeader pktHead;
        pktHead.to = receiverID;
        
        MailHeader mailHead;
        mailHead.to = mbox;
        mailHead.from = pkt.senderId;
        mailHead.length = strlen(message) + 1;
        // Send the message to other client
        bool retVal = postOffice->Send(pktHead, mailHead, message);
        if (!retVal) {
            printf("Cannot Send\n");
            interrupt->Halt();
        }else {
            //printf("Message sent: %s\n", message);
            fflush(stdout);
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
