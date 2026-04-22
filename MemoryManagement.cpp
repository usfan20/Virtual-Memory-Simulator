#pragma once
#include <iostream>
#include<list>
#include<unordered_map>
#include<deque>
#include "TLB & PTE.cpp"
#include "ReplacementAlgorithm.cpp"
#include "Config & Stats.cpp"

class MMU {
    SystemConfig config;
    StatsReport& stats;
    TLB tlb;
    unordered_map<uint32_t, PTE> pageTable;
    ReplacementAlgorithm* replacer;
    list<uint32_t> freeFrames;

    //handle page faults
    void handlePageFault(uint32_t vpn) {
        stats.pageFaults++;
        stats.totalTimeNs += config.diskLatency;

        uint32_t assignedFrame;
        if (!freeFrames.empty()) {
            assignedFrame = freeFrames.front();
            freeFrames.pop_front();
        }
        else {
            // 4. Eviction Logic 
            uint32_t victimVpn = replacer->getVictim();
            if (pageTable[victimVpn].dirty) {
                stats.diskWrites++;
                stats.totalTimeNs += config.diskLatency; // disk has to be accessed to write because dirty bit
            }
            assignedFrame = pageTable[victimVpn].frameNumber;
            pageTable[victimVpn].valid = false;
            tlb.invalidate(victimVpn);
        }

        pageTable[vpn] = { assignedFrame, true, false };
    }

public:
    MMU(SystemConfig c, StatsReport& s, ReplacementAlgorithm* r)
        : config(c), stats(s), tlb(c.tlbSize), replacer(r) {
        for (uint32_t i = 0; i < config.getTotalFrames(); ++i) freeFrames.push_back(i);
    }
    void access(uint32_t virtualAddress, bool isWrite) {
        stats.totalAccesses++; // every time it is called it total access are incremented
        uint32_t vpn = virtualAddress >> config.Shift();
        uint32_t offset = virtualAddress & config.Mask();
        int frame = tlb.lookup(vpn);
        stats.totalAccesses += config.tlbLatency;
        if (frame != -1) {
            stats.tlbHits++;
        }
        else { // Search if it exists in pagetable and if data is valid
            if (pageTable.count(vpn) && pageTable[vpn].valid)
            {
                stats.ptHits++;
                stats.totalTimeNs += config.ramLatency;
                frame = pageTable[vpn].frameNumber;
                tlb.update(vpn, frame);
            }
            else {  //pagefault
                handlePageFault(vpn);
                frame = pageTable[vpn].frameNumber;
                tlb.update(vpn, frame);

            }
        }
        if (isWrite) pageTable[vpn].dirty = true;
        replacer->updateUsage(vpn);


    }
};