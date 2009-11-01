#include "ipt.h"

using namespace std;

InvertedPageTable::InvertedPageTable(unsigned int pages) {
    entries = new InvertedPageTable[pages];
}

InvertedPageTable::~InvertedPageTable() {
    delete[] entries;
}

int InvertedPageTable::findPage(int vpn, int pid) {
    long long index = ((long long) vpn << 32) + pid;
    
    return pageMap.find(index);
}

InvertedPageTableEntry& InvertedPageTable::operator[](int ppn) {
    return entries[ppn];
}

