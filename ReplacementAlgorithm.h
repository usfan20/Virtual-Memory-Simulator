#pragma once
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <algorithm>
using namespace std;

class ReplacementAlgorithm {
public:
    virtual uint32_t getVictim() = 0;
    virtual void updateUsage(uint32_t vpn) = 0;
    virtual ~ReplacementAlgorithm() {}
};

class LRUAlgorithm : public ReplacementAlgorithm {
private:
    list<uint32_t> List;
    unordered_map<uint32_t, list<uint32_t>::iterator> lookupMap;
    size_t capacity;

public:
    LRUAlgorithm(size_t cap) : capacity(cap) {}

    void updateUsage(uint32_t vpn) override {
        // If already in list, remove old position
        if (lookupMap.find(vpn) != lookupMap.end()) {
            List.erase(lookupMap[vpn]);
        }
        // Insert at front as most recently used
        List.push_front(vpn);
        lookupMap[vpn] = List.begin();

        // Enforce capacity — remove LRU if over limit
        if (List.size() > capacity) {
            uint32_t lru = List.back();
            List.pop_back();
            lookupMap.erase(lru);
        }
    }

    uint32_t getVictim() override {
        if (List.empty()) {
            cerr << "Warning: LRU list is empty, cannot get victim!" << endl;
            return 0;
        }
        uint32_t victim = List.back();
        List.pop_back();
        lookupMap.erase(victim);
        return victim;
    }
};

class FIFOAlgorithm : public ReplacementAlgorithm {
    deque<uint32_t> fifoDeque;
    unordered_set<uint32_t> inMemory; // O(1) membership check
public:
    void updateUsage(uint32_t vpn) override {
        if (inMemory.find(vpn) == inMemory.end()) {
            fifoDeque.push_back(vpn);
            inMemory.insert(vpn);
        }
    }

    uint32_t getVictim() override {
        if (fifoDeque.empty()) {
            cerr << "Warning: FIFO queue is empty, cannot get victim!" << endl;
            return 0;
        }
        uint32_t victim = fifoDeque.front();
        fifoDeque.pop_front();
        inMemory.erase(victim);
        return victim;
    }
};

class OPTAlgorithm : public ReplacementAlgorithm {
    unordered_map<uint32_t, deque<int>>& futureUses;
    unordered_set<uint32_t> currentPagesSet; // O(1) lookup
    vector<uint32_t> currentPages;

public:
    OPTAlgorithm(unordered_map<uint32_t, deque<int>>& futures)
        : futureUses(futures) {
    }

    void updateUsage(uint32_t vpn) override {
        // Remove current usage from future timeline
        if (futureUses.count(vpn) && !futureUses[vpn].empty()) {
            futureUses[vpn].pop_front();
        }
        // Track page in memory
        if (currentPagesSet.find(vpn) == currentPagesSet.end()) {
            currentPages.push_back(vpn);
            currentPagesSet.insert(vpn);
        }
    }

    uint32_t getVictim() override {
        if (currentPages.empty()) {
            cerr << "Warning: OPT page list is empty, cannot get victim!" << endl;
            return 0;
        }

        uint32_t victim = currentPages[0];
        int furthestUse = -1;
        int victimIndex = 0;

        for (int i = 0; i < (int)currentPages.size(); ++i) {
            uint32_t vpn = currentPages[i];

            // Perfect victim — never used again
            if (!futureUses.count(vpn) || futureUses[vpn].empty()) {
                victim = vpn;
                victimIndex = i;
                break;
            }

            // Find page whose next use is furthest in future
            if (futureUses[vpn].front() > furthestUse) {
                furthestUse = futureUses[vpn].front();
                victim = vpn;
                victimIndex = i;
            }
        }

        currentPages.erase(currentPages.begin() + victimIndex);
        currentPagesSet.erase(victim);
        return victim;
    }
};