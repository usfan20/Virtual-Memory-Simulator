#pragma once
#include <iostream>
#include <list>
#include <unordered_map>
#include <deque>
#include "TLB & PTE.h"
#include "ReplacementAlgorithm.h"
#include "Config & Stats.h"

class MMU {
    SystemConfig config;
    StatsReport& stats;
    TLB tlb;
    unordered_map<uint32_t, PTE> pageTable;
    ReplacementAlgorithm* replacer;
    list<uint32_t> freeFrames;

    void handlePageFault(uint32_t vpn) {
        stats.pageFaults++;
        stats.totalTimeNs += config.diskLatency;

        uint32_t assignedFrame;
        if (!freeFrames.empty()) {
            assignedFrame = freeFrames.front();
            freeFrames.pop_front();
        }
        else {
            // Eviction Logic
            uint32_t victimVpn = replacer->getVictim();

            // Safety check — victim must exist in page table
            if (pageTable.find(victimVpn) == pageTable.end()) {
                cerr << "Warning: Victim VPN not found in page table!" << endl;
                assignedFrame = 0; // fallback
            }
            else {
                if (pageTable[victimVpn].dirty) {
                    stats.diskWrites++;
                    stats.totalTimeNs += config.diskLatency; // dirty write-back penalty
                }
                assignedFrame = pageTable[victimVpn].frameNumber;
                pageTable[victimVpn].valid = false;
                tlb.invalidate(victimVpn);
            }
        }

        // Load new page into assigned frame
        pageTable[vpn] = { assignedFrame, true, false };
    }

public:
    MMU(SystemConfig c, StatsReport& s, ReplacementAlgorithm* r)
        : config(c), stats(s), tlb(c.tlbSize), replacer(r) {
        for (uint32_t i = 0; i < config.getTotalFrames(); ++i)
            freeFrames.push_back(i);
    }

    void access(uint32_t virtualAddress, bool isWrite) {
        stats.totalAccesses++;

        uint32_t vpn = virtualAddress >> config.Shift();
        uint32_t offset = virtualAddress & config.Mask();
        // Offset extracted for completeness; physical address = frame * pageSize + offset

        int frame = tlb.lookup(vpn);

        if (frame != -1) {
            // TLB Hit — fastest path
            stats.tlbHits++;
            stats.totalTimeNs += config.tlbLatency;
        }
        else {
            // TLB Miss — must search page table
            stats.totalTimeNs += config.tlbLatency; // TLB miss still costs 1ns

            if (pageTable.count(vpn) && pageTable[vpn].valid) {
                // Page Table Hit
                stats.ptHits++;
                stats.totalTimeNs += config.ramLatency;
                frame = pageTable[vpn].frameNumber;
                tlb.update(vpn, frame);
            }
            else {
                // Page Fault
                stats.totalTimeNs += config.ramLatency; // loading page into RAM
                handlePageFault(vpn);
                frame = pageTable[vpn].frameNumber;
                tlb.update(vpn, frame);
            }
        }

        if (isWrite) pageTable[vpn].dirty = true;
        replacer->updateUsage(vpn);
    }
};