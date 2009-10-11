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
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

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
        currentThread->space->killAllThreads();
        return;
    }
    machine->Run();
}

void Fork_Syscall(int funcAddr){
    DEBUG('a', "%s: Called Fork_Syscall.\n",currentThread->getName());
        // Create new thread.kernel_thread()
    Thread *t = new Thread(currentThread->getName());
    // Stack was successfully created.
    // Add Process to the system's process table.
    t->setPID(processTable->addProcess(t));
    currentThread->space->addChildThread(t->getPID());
        // Address space for new Thread and the spawning thread is the same.
    t->space = currentThread->space;
        // Fork to get the new thread.
    t->Fork((VoidFunctionPtr) kernel_thread, funcAddr);
        // Restore state.
    currentThread->space->RestoreState();
}

void exec_thread(int arg){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
        // We should never reach here.
}

spaceId Exec_Syscall(char *filename){
    DEBUG('a', "%s: Called Exec_Syscall\n",currentThread->getName());
        // Get filename to kernel space
    std::string cname = currentThread->space->readCString(filename);
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());
    cout << "Exec_Syscall: Arg: " <<c_name<<endl;
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
    t->setPID(processTable->addProcess(t));
    currentThread->space->addChildThread(t->getPID());
    DEBUG('a', "%s: New thread created with PID: %d.\n",currentThread->getName(), t->getPID());
    
    t->Fork((VoidFunctionPtr) exec_thread, 0);
    return (spaceId) t->space;
}

void Exit_Syscall(int status){
    cout <<currentThread->getName()<<": Exit status: "<<status<<endl;
    if (processTable->getProcessCount() == 1 && currentThread->space->childThreads.size() == 0) {
        DEBUG('a', "Exit_Syscall:End of all processes across NACHOS\n");
        interrupt->Halt();
    }else if (currentThread->space->childThreads.size() == 0 && processTable->getProcessCount()>1) {
        DEBUG('a', "Exit_Syscall:End of Process PID: %d\n", currentThread->getPID());
        currentThread->space->removeChildThread(currentThread->getPID());
        currentThread->Finish();
    }else {
            //Neither the end of process nor the end of Nachos
        DEBUG('a',"Exit_Syscall:End of Thread PID: %d\n", currentThread->getPID());
        currentThread->space->removeChildThread(currentThread->getPID());
        currentThread->Finish();
    }
}


void Yield_Syscall(){
    (void) currentThread->Yield();
}

LockId CreateLock_Syscall(char* name){
    DEBUG('a',"%s: CreateLock_Syscall initiated.\n", currentThread->getName());
    std::string cname = currentThread->space->readCString(name);
    cout<<"Lock Name: "<<cname;
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());
    LockWrapper *newLock = new LockWrapper(new Lock(c_name), 0, false);
        //    Lock *newLock = new Lock(c_name);    
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
}

void AcquireLock_Syscall(LockId lockId){
    LockWrapper *targetLock = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    if(targetLock == NULL){
        DEBUG('a',"%s: AcquireLock_Syscall: Unable to find lock %d for acquire.\n",
              currentThread->getName(), lockId);
        return;
    }else{
        DEBUG('a',"%s: Lock %d: AcquireLock_Syscall.\n",currentThread->getName(),lockId);
        targetLock->lock->Acquire();
        targetLock->counter++;
        return;
    }
}

void ReleaseLock_Syscall(LockId lockId){
    LockWrapper *targetLock = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    if(targetLock == NULL){
        DEBUG('a',"%s: AcquireLock_Syscall: Unable to find lock %d for acquire.\n",
              currentThread->getName(), lockId);
        return;
    }else{
        DEBUG('a',"%s: Lock %d: ReleaseLock_Syscall.\n",currentThread->getName(),lockId);
        targetLock->lock->Release();
        targetLock->counter--;
        
        if(targetLock->mark && targetLock->counter == 0){
                //Check for deletion
            delete targetLock->lock;
            delete targetLock;
            DEBUG('a',"%s: ReleaseLock_Syscall: Successfully deleted lock %d .\n",
                  currentThread->getName(), lockId);
        } 
    }
}


CVId CreateCondition_Syscall(char* name){
	DEBUG('a',"%s : CreateCondition_Syscall initialized.\n",currentThread->getName());
    std::string cname = currentThread->space->readCString(name);
    cout<<"Condition Name: "<<cname;
    char *c_name = new char[cname.size()+1];
    strcpy(c_name, cname.c_str());    
    
	ConditionWrapper *newCV = new ConditionWrapper(new Condition(c_name), 0, false);
	
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
    DEBUG('a', "DestroyCondition syscall.\n");
	ConditionWrapper *target = (ConditionWrapper*) currentThread->space->CVTable.Get(id);
    if(target){
        if (target->mark && target->counter == 0) {
                //We can delete
            delete target->cv;
            delete target;
            DEBUG('a',"%s : DestroyCondition_Syscall: Successfully deleted CV %d .\n",
                  currentThread->getName(), id);
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
	currentThread->space->CVTable.Remove(id);
}

void WaitCV_Syscall(CVId cvId, LockId lockId){
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    
    if (ConditionLockWrapper == NULL || CV == NULL) {
        return;
    }
        //Set wait on this condition variable
    DEBUG('a',"%s: WaitCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    CV->counter++;
    (void) CV->cv->Wait(ConditionLockWrapper->lock);
}

void SignalCV_Syscall(CVId cvId, LockId lockId){
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    
    if (ConditionLockWrapper == NULL || CV == NULL) {
        return;
    }
    
        //Set wait on this condition variable
    DEBUG('a',"%s: SignalCV_Syscall: Called for CVId: %d lockId %d .\n", 
          currentThread->getName(), cvId, lockId);
    (void) CV->cv->Signal(ConditionLockWrapper->lock);
    CV->counter--;
    if (CV->mark && CV->counter == 0) {
            //We can delete
        delete CV->cv;
        delete CV;
        DEBUG('a',"%s : DestroyCondition_Syscall: Successfully deleted CV %d .\n",
              currentThread->getName(), cvId);
    }else {
            //Cannot delete need to persist just mark for deletion
        CV->mark = true;
        DEBUG('a',"%s: DestroyCondition_Syscall: marked CV %d for deletion.\n",
              currentThread->getName(), cvId);
    }
}

void BroadcastCV_Syscall(CVId cvId, LockId lockId){
    LockWrapper *ConditionLockWrapper = (LockWrapper*) currentThread->space->locksTable.Get(lockId);
    ConditionWrapper *CV = (ConditionWrapper*) currentThread->space->CVTable.Get(cvId);
    
    if (ConditionLockWrapper == NULL || CV == NULL) {
        return;
    }
    
        //Set wait on this condition variable
    DEBUG('a',"%s: WaitCV_Syscall: Called for CVId: %d lockId %d .\n",
          currentThread->getName(), cvId, lockId);
    (void) CV->cv->Broadcast(ConditionLockWrapper->lock);
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
