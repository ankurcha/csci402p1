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

using namespace std;
extern "C" { int bzero(char *, int); };
Lock *processTableLock = new Lock("processTableLock");
Lock* locksTableLock = new Lock("locksTableLock");
Lock* CVTableLock = new Lock("CVTableLock");
int TLBIndex = 0;
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
/*
bool ReplaceSwapFilePage(int pageframe){
    char *buf; // Buffer
    int pos; // Offset into the swap file.
    int SwapPage = -1; // Location of the first open swap page
    int bytesRead; // Number of bytes read
    int IPTnum;
    int virtualAddr, physAddr;
    unsigned int pageoffset; // page offset
    unsigned int swapOffset; // swap offset

    virtualAddr = IPT[pageframe].virtualPage * PageSize;
    pageoffset = virtualAddr % PageSize;

    physAddr = IPT[pageframe].physicalPage * PageSize + pageoffset;

    swapOffset = currentThread->PID * UserStackSize;
    // Find the corresponding page in the swap file data structure
    for(unsigned int i=0; i<(swapOffset+UserStackSize); i++){
        if(Swap[i].virtualPage == IPT[pageframe].virtualPage &&
                Swap[i].PID == currentThread->PID){
            SwapPage = i;
            break;
        }
    }

    if(SwapPage == -1){
        DEBUG('a', "Error: Swap full!\n");
        return false;
    }

    // Calculate Virtual Address and Position

    pos = SwapPage * PageSize;
    buf = (char*) (machine->mainMemory + physAddr);

    for(int i=0;i<PageSize; i++){
        bytesRead = swapFile->WriteAt(buf+i, 1, pos+i);

        if(bytesRead == -1){
            DEBUG('a', "Cannot write to Physical Address: %d into swap file.\n", pos+i);
            return false;
        }
    }

    Swap[SwapPage].physicalPage = SwapPage;
    Swap[SwapPage].virtualPage = IPT[pageframe].virtualPage;
    Swap[SwapPage].PID = currentThread->PID;
    swapBitMap->Mark(SwapPage);
    return true;
}

bool ReadSwap(int virtPage, char *buf){
    TranslationEntry page;
    int pos;
    bool found = false;
    int bytesRead;
    int offset = currentThread->PID * UserStackSize;
    for(int i = offset; i<(offset+UserStackSize); i++){
        if(Swap[i].virtualPage == virtPage &&
                currentThread->PID == Swap[i].PID){
            page.virtualPage  = Swap[i].virtualPage;
            page.physicalPage = Swap[i].physicalPage;
            page.valid = Swap[i].valid;
            page.readOnly = Swap[i].readOnly;
            page.use = Swap[i].use;
            page.dirty = Swap[i].dirty;
            
            found = true;
            DEBUG('a', "Found page in IPT\n");
            break;
        }
    }

    if(found){
            // Check if it is in the swap file.
        if(swapBitMap->Test(page.physicalPage)){
            // Bitmap is set.
            pos = page.physicalPage * PageSize;
            for( int i=0; i< PageSize; i++){
                bytesRead = swapFile->ReadAt(buf+i, 1, pos+i);
                if(bytesRead == -1){
                    return false;
                }
            }
        }else{
            return false;
        }
    }else{
        return false;
    }
    return true;
}

bool WriteTWriteToSwapoSwap(int physPageNum) {
    char* buf; //Final buffer containing page to write to swap
    int pos; //Byte offset in swapfile
    int openSwapPage = -1; //location of the first open swap page number
    int bytesRead; //Number of bytes successfully read
    int virtualAddr, physicalAddr;
    unsigned int pageOffset;// page offset
    int swapOffset;
    
    virtualAddr = IPT[physPageNum]virtualPage * PageSize;
    pageOffset = virtualAddr % PageSize;
    
    physicalAddr = IPT[physPageNum].physicalPage * PageSize + pageOffset;
    
    swapOffset = currentThread->space->PID * UserStackSize;
    // Find next available slot in swap and set it
    for(int i=swapOffset; i<(swapOffset+UserStackSize); i++) {
        if(!swapBitMap->Test(i)) {
            // Found an unused swap page
            openSwapPage = i;
            break;
        }
    }
    
    if(openSwapPage == -1) { // Out of swap space!!
        DEBUG('a', "Error: SWAP is full!\n");
        return false;
    }
    
    //Calculate Virtual Address and Position
    pos = openSwapPage * PageSize;
    // Point to the memory location to be sent to swap
    buf = (char *)(machine->mainMemory+physicalAddr);
    
    // Write things to swap actually!!
    for(int i=0; i<PageSize; i++) {
        bytesRead = swapFile->WriteAt(buf+i, 1, pos+i);
        if(bytesRead == -1) {
            DEBUG('a', "Error: Could not write to pAddr: %d into swap file\n", pos+i);
            return false;
        }
    }
    
    //Update the Swap Data Structure
    Swap[openSwapPage].physicalPage = openSwapPage;
    Swap[openSwapPage].virtualPage = IPT[physPageNum].virtualPage;
    Swap[openSwapPage].PID = currentThread->PID;
    // Mark up this swap space as 'taken'
    swapBitMap->Mark(openSwapPage);
    return true;
}


void handlePageFaultException(int badVAddr){
    IntStatus oldStatus = interrupt->SetLevel(IntOff);
    stats->numPageFaults++;
    InvertedPageTableEntry Page;
    
    int virtPageIndex = badVAddr / PageSize;
    
    if(badVAddr < 0){
        DEBUG('a',"Illegal virtual address in page fault: %d\n",badVAddr);
        currentThread->Finish();
    }
    bool found = false;
        //Search for the corresponding IPT entry
    for (int i=0; i<NumPhysPages; i++) {
        if (IPT[i].virtualPage == virtPageIndex && IPT[i].valid == TRUE) {
            Page = IPT[i];
            found = true;
            DEBUG('a', "Found in IPT!\n");
            break;
        }
    }
    int newPage = -1;
    if (!found) {
            //Choose a new page
        for (int i=0; i<NumPhysPages; i++) {
            if (!IPT[i].valid) {
                newPage = i;
                break;
            }
        }
        if (newPage<0) {
                //Full!
            if (!FIFOreplacementPolicy) {
                    //Do Random Replacement
                newPage = Random() % NumPhysPages;
            }else {
                    //FIFO replacement
                newPage = 0;
                for (int i=0; i<NumPhysPages; i++) {
                    if (IPT[i].age < IPT[newPage].age) {
                        newPage = i;
                    }
                }
            }
            DEBUG('a', "No empty pages in IPT replacing page %d for VADDR %d\n", newPage, badVAddr);
            bool inSwapFile = false;
            for (int i=0; i<NumPhysPages; i++) {
                if (Swap[i].virtualPage == IPT[newPage].virtualPage && swapBitMap->Test(i)) {
                    inSwapFile = true;
                    break;
                }
            }
            
            if(inSwapFile){
                DEBUG('a', "Replacing page %d in swap file\n", newPage);
                ReplaceSwapFilePage(newPage);
            }else{
                DEBUG( 'a', "Writing page %d into swap file\n", newPage);
                WriteToSwap(newPage);
            }
        IPT[newPage].valid = false; //Invalidate a swapped page
        }else {
                // found an empty slot put the page there.
            DEBUG('a', "Place page in slot %d", newPage);
        }
    }
    char buf[PageSize];
    if (!found && ReadSwap(virtPageIndex, buf)) {
        found = true;
        DEBUG('S', "Load page from swap into main memory\n");
        for (int i=0; i<PageSize; i++) {
            machine->mainMemory[newPage * PageSize +i] = buf[i];
        }
        IPT[newPage].virtualPage = virtPageIndex;
        IPT[newPage].physicalPage = newPage;
        IPT[newPage].valid = TRUE;
        IPT[newPage].use = FALSE;
        IPT[newPage].dirty = FALSE;
        IPT[newPage].readOnly = FALSE;
        IPT[newPage].age = curAge++;
        Page = IPT[newPage];
        DEBUG('S', "DONE!\n");
    }
    
    if (!found) {
            //Load From executable
        DEBUG('a', "Not in swap, load page from executable\n");
        int num = currentThread->space->loadVirtualPage(badVAddr, newPage);
        if (num > -1) {
            DEBUG('a', "Loaded physical page %d from executable\n",num);
            Page = IPT[num];
            found = true;
        }
        WriteToSwap(num);
        IPT[num].age = curAge++;
    }
    
    if(!found){
        DEBUG('a', "Cannot find virtual page %d in IPT\n", virtPageIndex);
        if(++faults >13){
            DEBUG('a', "Killing Thread to avoid INF loop\n");
            for (int i = 0; i<NumPhysPages; i++) {
                if (IPT[i].virtualPage > 0) {
                    DEBUG('a', "IPT: %d virtual: %d Physical: %d\n",i, IPT[i].virtualPage, IPT[i].physicalPage);
                }
            }
            faults = 0;
            currentThread->Finish();
        }
        return;
    }
        //Convert Page to Translation Entry page
    
    TranslationEntry page;
    page.virtualPage = Page.virtualPage;
    page.physicalPage = Page.physicalPage;
    page.valid = Page.valid;
    page.use = Page.use;
    page.dirty = Page.dirty;
    page.readOnly = Page.readOnly;
    
    machine->tlb[TLBIndex] = page;
    TLBIndex++;
    TLBIndex = TLBIndex % 4;
    
    (void) interrupt->SetLevel(oldStatus);   
}

int loadPage(int VADDR){

    int VirtualPageNum = VADDR / PageSize;
    DEBUG('a',"Loading VADDR %d from PAGE# %d\n",VADDR,VirtualPageNum);
    // Find the corresponding page in the IPT corresponding to the VPN
    for(int i=0; i<NumPhysPages; i++){
        if(IPT[i].virtualPage == VirtualPageNum){
            return i;
        }
    }
    // Page wasn't in memory, handle Page fault
    handlePageFaultException(VirtualPageNum * PageSize);
    for(int i=0; i<NumPhysPages; i++){
        if(IPT[i].virtualPage == VirtualPageNum){
            return i;
        }
    }

    // We should not reach here!!
    DEBUG('a', "BIG PROBLEM PAGE WAS NOWHERE TO BE SEEN\n");
    interrupt->Halt();
    return -1;
}
*/

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
    currentThread->space->childThreads++;
    t->setPID(currentThread->space->childThreads);
        // Address space for new Thread and the spawning thread is the same.
    t->space = currentThread->space;
    t->Fork((VoidFunctionPtr) kernel_thread, funcAddr);
        // Restore state.
    currentThread->space->RestoreState();
    processTableLock->Release();
}

void exec_thread(int arg){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
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
    processTable->processCounter++;
    t->setPID(processTable->processCounter);
    DEBUG('a', "%s: New thread created with PID: %d.\n",currentThread->getName(), t->getPID());
    processTableLock->Release();
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

void handlePageFaultException(int vAddr){
    int virtualpage = vAddr / PageSize;
    int physicalPage;
    DEBUG('a', "Handling PageFault for VADDR: %d\n", vAddr);
    TLBIndex = (TLBIndex+1) % TLBSize;
    
    // Copy out all pages from the TLB - update the IPT
    for (int i=0; i<TLBSize; i++){
        if(machine->tlb[i].valid){
            IPT[machine->tlb[i].physicalPage].virtualPage = machine->tlb[i].virtualPage;
            IPT[machine->tlb[i].physicalPage].physicalPage = machine->tlb[i].physicalPage;
            IPT[machine->tlb[i].physicalPage].valid = machine->tlb[i].valid;
            IPT[machine->tlb[i].physicalPage].use = machine->tlb[i].use;
            IPT[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
            IPT[machine->tlb[i].physicalPage].readOnly = machine->tlb[i].readOnly;
        }
    }

    // Now we check if the currentThread->space pageTable is in memory
    // If yes, load from IPT

    if(currentThread->space->PageTableInfo[virtualpage].PageStatus == MEMORY){
        // Copy page table to TLB from IPT
        for(int i=0; i< NumPhysPages; i++){
            if(IPT[i].virtualPage == vAddr && IPT[i].PID == currentThread->PID){
                physicalPage = i;
                break;
            }
        }
        DEBUG('a',"Physical Page: %d\n",physicalPage);
        // Copy IPT -> TLB
        machine->tlb[TLBIndex].virtualPage = IPT[physicalPage].virtualPage;
        machine->tlb[TLBIndex].physicalPage = IPT[physicalPage].physicalPage;
        machine->tlb[TLBIndex].valid = IPT[physicalPage].valid;
        machine->tlb[TLBIndex].use = IPT[physicalPage].use;
        machine->tlb[TLBIndex].dirty = IPT[physicalPage].dirty;
        machine->tlb[TLBIndex].readOnly = IPT[physicalPage].readOnly;
        return;
    }

    // Find a free page in memory to get th page in - FindOpenPhysicalPage
    physicalPage = -1;
    for(int i=0;i<NumPhysPages;i++){
        if(IPT[i].physicalPage == -1){
            // This page is free!!!
            DEBUG('a', "Found a free page at %d\n", i);
            physicalPage = i;
            break;
        }
    }

    if(physicalPage == -1){
        // All pages are full - time to swap pages
        if(!FIFOreplacementPolicy){
            physicalPage = Random() % NumPhysPages;
        }else{
            for(int i=0; i<NumPhysPages; i++){
                if(IPT[i].age < IPT[physicalPage].age){
                    physicalPage = i;
                }
            }
        }

        virtualpage = IPT[physicalPage].virtualPage;
        if(IPT[physicalPage].dirty){
            // Page modified but not committed.
            if(IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation == -1){
                IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation = swapLocation++;
            }
            // Write the page to swapLocation.
            swapFile->WriteAt(&(machine->mainMemory[PageSize * physicalPage]),PageSize,
                    (PageSize * IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation));
            IPT[physicalPage].space->PageTableInfo[virtualpage].PageStatus = SWAP;
        }else if(IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation == -1){
            // Okay!! page is not dirty and was never written to swap file...
            IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation = swapLocation++;
            swapFile->WriteAt(&(machine->mainMemory[PageSize * physicalPage]),PageSize,
                    (PageSize * IPT[physicalPage].space->PageTableInfo[virtualpage].swapLocation));
            IPT[physicalPage].space->PageTableInfo[virtualpage].PageStatus = SWAP;
        }else{
            // Not dirty and already in the swap file. Don't touch it!!
            // Just update the PageStatus
            IPT[physicalPage].space->PageTableInfo[virtualpage].PageStatus = SWAP;
        }

        //If currentThread, invalidate all entries in TLB
        if(IPT[physicalPage].space == currentThread->space){
            for(int i=0; i< TLBSize; i++){
                if(machine->tlb[i].virtualPage == virtualpage){
                    machine->tlb[i].valid = false;
                }
            }
        }
    }
    // So by now we have the physical page which we want to screw with.
    // so, zero it out!!
    bzero(&(machine->mainMemory[PageSize * physicalPage]), PageSize);

    if(currentThread->space->PageTableInfo[virtualpage].PageStatus == EXEC){
        // Load from file!
        DEBUG('a', "Loading page from file: %d, virtualPage: %d\n", currentThread->space->noffH.code.inFileAddr, virtualpage);
        currentThread->space->executable->ReadAt(&(machine->mainMemory[physicalPage * PageSize]), PageSize, currentThread->space->noffH.code.inFileAddr + virtualpage * PageSize);
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
    machine->tlb[TLBIndex].virtualPage = IPT[physicalPage].virtualPage;
    machine->tlb[TLBIndex].physicalPage = IPT[physicalPage].physicalPage;
    machine->tlb[TLBIndex].valid = IPT[physicalPage].valid;
    machine->tlb[TLBIndex].use = IPT[physicalPage].use;
    machine->tlb[TLBIndex].dirty = IPT[physicalPage].dirty;
    machine->tlb[TLBIndex].readOnly = IPT[physicalPage].readOnly;
    // Everything is done!!
    return;
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
