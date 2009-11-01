#ifndef __PROCESSTABLE_H__
#define __PROCESSTABLE_H__

#include "synch.h"
#include <map>
#include <iostream>
using namespace std;

struct Process {
    Lock *childLock;
    Condition *childCV;
    int childCount;
    
    PID pid;
    PID ppid;
    Process(){
        childLock = new Lock("");
        childCV = new Condition("");
        childCount = 0;
    }
    ~Process(){
        delete childCV;
        delete childLock;
    }
};

struct ProcessTable {
private:
    Lock* tableLock;
    map<PID, Process*> *table;
public:
    int processCounter;
    PID addProcess(PID PPID){
        Process *process = new Process();
        Process *parentProcess = NULL;
        
        process->ppid = PPID;
        tableLock->Acquire();
        PID pid = processCounter++;
        process->pid = pid;
        table->insert(pair<char,Process*>(pid,process));
        if (PPID>=0) {
            parentProcess = getProcess(PPID);
            if (parentProcess != NULL) {
                parentProcess->childLock->Acquire();
                parentProcess->childCount++;
                parentProcess->childLock->Release();
            }
        }
        tableLock->Release();
        return pid;
    }
    
    int removeProcess(PID pid){
        Process *process = getProcess(pid);
        Process *parentProcess;
        if(process != NULL){
            process->childLock->Acquire();
            if (process->childCount != 0) {
                // Wait for all process to exit, avoid zombies
                process->childCV->Wait(process->childLock);
            }
            
            if (process->ppid >= 0) {
                parentProcess = getProcess(process->ppid);
                if (parentProcess!= NULL) {
                    parentProcess->childLock->Acquire();
                    // Actually remove process from process table
                    table->erase(table->find(pid));
                    parentProcess->childCount--;
                    if (parentProcess->childCount == 0) {
                        parentProcess->childCV->Signal(parentProcess->childLock);
                    }
                    parentProcess->childLock->Release();
                    process->childLock->Release();
                    return 1;
                }
            }else {
                table->erase(table->find(pid));
                process->childLock->Release();
                return 0;
                }
            
        }
        return -1;
    }
    
    
    Process* getProcess(PID pid){
        map<PID, Process*>::iterator it;
        it = table->find(pid);
        if (it == table->end()) {
            // Unable to find process
            return NULL;
        }else {
            return (Process *) table->find(pid)->second;
        }
    }
    
    ProcessTable(){
        tableLock = new Lock("ProcessTableLock");
        table = new map<PID, Process*>();
        processCounter = 0;
    }
    
    ~ProcessTable(){
        delete tableLock;
        delete table;
        processCounter = 0;
    }
    
};

#endif

