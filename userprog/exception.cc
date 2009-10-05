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
#include <stdio.h>
#include <iostream>

using namespace std;

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
        result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );
        
        if ( !result ) {
                //translation failed
            return -1;
        }
        
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

    //-------------PENDING SYSCALLS-------------//
void kernel_thread(int virtAddr){
        //Setup new Thread
    machine->WriteRegister(PCReg, virtAddr);
    machine->WriteRegister(NextPCReg, virtAddr+4);
    currentThread->space->RestoreState();
        //TODO: Create a new Stack for this thread and write virtAddr to
        // StackRegister - MAX's Voodoo
        machine->WriteRegister(StackReg, virtAddr);
    machine->Run();
}

void Fork_Syscall(int funcAddr){
    DEBUG('a', "%s: Called Fork_Syscall.\n",currentThread->getName());
        // TODO: Waiting for Max to give insight into the organizaton of the
        // code in the address space
    Thread *t = new Thread(currentThread->getName());
    //TODO: Update process Table and related Data structures - Max's Voodoo :)
    currentThread->space->addChildThread();
    t->space = currentThread->space;
    t->Fork((VoidFunctionPtr) kernel_thread, funcAddr);
    currentThread->space->RestoreState();
    return;
}

void exec_thread(int arg){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
    
    ASSERT(FALSE);
}

spaceId Exec_Syscall(char *filename){
    DEBUG('a', "%s: Called Exec_Syscall\n",currentThread->getName());
        // TODO: Waiting for Max to give insight into the organizaton of the
        // code in the address space
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;
    
    if(!executable){
        DEBUG('a',"%s: Unable to open file %s .\n", currentThread->getName(),
              filename);
        return -1;
    }
    Thread *t = new Thread(filename);
    t->space = new AddrSpace(executable);
        //Add Thread to processTable and get the PID
    t->setPID(processTable->addProcess(t));
    DEBUG('a', "%s: New thread created with PID: %d.\n",currentThread->getName(),
          t->getPID());
    t->Fork((VoidFunctionPtr) exec_thread, 0);
    return (spaceId) t->space;
}

void Exit_Syscall(int status){
    cout <<currentThread->getName()<<": Exit status: "<<status<<endl;
    if (processTable->getProcessCount() == 1 && currentThread->space->numChildThreads == 0) {
        DEBUG('a', "End of all processes across NACHOS\n");
        interrupt->Halt();
    }else if (currentThread->space->numChildThreads == 0 && processTable->getProcessCount()>1) {
        DEBUG('a', "End of Process PID: %d\n", currentThread->getPID());
        currentThread->space->removeChildThread();
        processTable->killProcess(currentThread->getPID());
        currentThread->Finish();
    }else {
            //Neither the end of process nor the end of Nachos
        DEBUG('a',"End of Thread PID: %d\n", currentThread->getPID());
        currentThread->space->removeChildThread();
        processTable->killProcess(currentThread->getPID());
        currentThread->Finish();
    }
}

    //----------------------------------------//

void Yield_Syscall(){
    (void) currentThread->Yield();
}

LockId CreateLock_Syscall(char* name){
    DEBUG('a',"%s: CreateLock_Syscall initiated.\n", currentThread->getName());
    Lock *newLock = new Lock(name);
    int retval;
    if (newLock) {
        if ((retval = currentThread->space->locksTable.Put(newLock)) == -1) {
                //unable to put the lock into the locktable
            delete newLock;
        }else {
            return retval;
        }
    }
        //Error!!!
    return -1;
}

void DestroyLock_Syscall(LockId id){
    Lock *targetLock = (Lock*) currentThread->space->locksTable.Get(id);
    if (targetLock) {
            //Found the lock, Now delete it
        delete targetLock;
        DEBUG('a',"%s: DestroyLock_Syscall: Successfully deleted lock %d .\n",
              currentThread->getName(), id);
    }else {
            //Unable to find targetLock
        DEBUG('a',"%s: DestroyLock_Syscall: Unable to find lock %d for deletion.\n",
              currentThread->getName(), id);
    }
    currentThread->space->locksTable.Remove(id);
}

void AcquireLock_Syscall(LockId lockId){
    Lock *targetLock = (Lock*) currentThread->space->locksTable.Get(lockId);
    ASSERT((targetLock != NULL));
    
    DEBUG('a',"%s: Lock %d: AcquireLock_Syscall.\n",currentThread->getName(),lockId);
    targetLock->Acquire();
}

void ReleaseLock_Syscall(LockId lockId){
    Lock *targetLock = (Lock*) currentThread->space->locksTable.Get(lockId);
    ASSERT((targetLock != NULL));
    
    DEBUG('a',"%s: Lock %d: ReleaseLock_Syscall.\n",currentThread->getName(),lockId);
    targetLock->Release();
}


CVId CreateCondition_Syscall(char* name){
	DEBUG('a',"%s : CreateCondition_Syscall initialized.\n",currentThread->getName());
	Condition *newCV = new Condition(name);
	int retval;
	if(newCV){
		if((retval = currentThread->space->CVTable.Put(newCV)) == -1){
			delete newCV;
		}else{
			return retval;
		}
	}
    
	return -1;
}

void DestroyCondition_Syscall(CVId id){
	Condition *target = (Condition*) currentThread->space->CVTable.Get(id);
	if(target){
		delete target;
		DEBUG('a',"%s : DestroyCondition_Syscall: Successfully deleted CV %d .\n",
              currentThread->getName(), id);
	}else{
		DEBUG('a',"%s: DestroyCondition_Syscall: Unable to find CV %d for deletion.\n",
              currentThread->getName(), id);
	}
	currentThread->space->CVTable.Remove(id);
}

void WaitCV_Syscall(CVId cvId, LockId lockId){
    Lock *ConditionLock = (Lock*) currentThread->space->locksTable.Get(lockId);
    Condition *CV = (Condition*) currentThread->space->CVTable.Get(cvId);
    
    ASSERT((ConditionLock != NULL && CV!= NULL));
    
        //Set wait on this condition variable
    DEBUG('a',"%s: WaitCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    (void) CV->Wait(ConditionLock);
}

void SignalCV_Syscall(CVId cvId, LockId lockId){
    Lock *ConditionLock = (Lock*) currentThread->space->locksTable.Get(lockId);
    Condition *CV = (Condition*) currentThread->space->CVTable.Get(cvId);
    
    ASSERT((ConditionLock != NULL && CV!= NULL));
    
        //Set wait on this condition variable
    DEBUG('a',"%s: SignalCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    (void) CV->Signal(ConditionLock);
}

void BroadcastCV_Syscall(CVId cvId, LockId lockId){
    Lock *ConditionLock = (Lock*) currentThread->space->locksTable.Get(lockId);
    Condition *CV = (Condition*) currentThread->space->CVTable.Get(cvId);
    
    ASSERT((ConditionLock != NULL && CV!= NULL));
    
        //Set wait on this condition variable
    DEBUG('a',"%s: WaitCV_Syscall: Called for CVId: %d lockId %d .\n",
          currentThread->getName(), cvId, lockId);
    (void) CV->Broadcast(ConditionLock);
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
                    //Incomplete------------
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
                    //----------------------
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
#endif
        }
        
            // Put in the return value and increment the PC
        machine->WriteRegister(2,rv);
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
        return;
    } else {
        cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
        interrupt->Halt();
    }
}
