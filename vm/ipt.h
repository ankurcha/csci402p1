// Inverted Page Table class
//
// USC CS 402 Group 11
// Fall 2009

#ifndef VM_IPT_H
#define VM_IPT_H 1

#include <map>

class InvertedPageTableEntry : public TranslationEntry{
public:
    InvertedPageTableEntry(){
            virtualPage = -1;
            physicalPage = -1;
            age = 0;
            valid = false;
            dirty = false;
        }
    enum {
            CODE,
            DATA,
            OTHER
        } ContentType;
    enum {
            MEMORY,
            EXEC,
            SWAP,
        }Location;

    int PID;    // TO find out who is the owner
    int age;    // Used for FIFO replacement
    AddrSpace *space; // To keep track of which pages we are refering to.
    status PageStatus;
    int swapLocation;
};

class InvertedPageTable {
public:
    InvertedPageTable(unsigned int pages);
    ~InvertedPageTable();

    int findEmptyPage() const;
    int findPage(int vpn, int pid) const;
    InvertedPageTableEntry& operator[](int ppn) const;

private:
    InvertedPageTableEntry* entries;
    std::map<long long, int> pageMap;
}

#endif // VM_IPT_H

