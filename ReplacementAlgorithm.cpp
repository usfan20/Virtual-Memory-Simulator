#pragma once
#include <iostream>
#include<list>
#include<unordered_map>
#include<deque>
using namespace std;

class ReplacementAlgorithm {
public:
    virtual uint32_t getVictim() = 0;
    virtual void updateUsage(uint32_t vpn) = 0;
    virtual ~ReplacementAlgorithm() {}
};
class LRUAlgorithm : public ReplacementAlgorithm {
private:
    std::list<uint32_t> List;
    std::unordered_map<uint32_t, list<uint32_t>::iterator> lookup;
    size_t capacity;

public:
    LRUAlgorithm(size_t cap) : capacity(cap) {}

    // Step 1: Update the timeline whenever a page is accessed
    void updateUsage(uint32_t vpn) override {
        // remove its old position
        if (lookup.find(vpn) != lookup.end()) {
            List.erase(lookup[vpn]);
        }

        // move it front most used
        List.push_front(vpn);

        // Update the location
        lookup[vpn] = List.begin();
    }

    // Step 2: Identify the victim when we need to free up a frame
    uint32_t getVictim() override {
        uint32_t victim = List.back();

        List.pop_back();
        lookup.erase(victim);

        return victim;
    }
};
class FIFOAlgorithm : public ReplacementAlgorithm {
    deque<uint32_t> fifoDeque;

public:
    void updateUsage(uint32_t vpn) override {

        deque<uint32_t>::iterator it = find(fifoDeque.begin(), fifoDeque.end(), vpn);


        if (it == fifoDeque.end()) {
            fifoDeque.push_back(vpn);
        }
    }

    uint32_t getVictim() override {
        uint32_t victim = fifoDeque.front();
        fifoDeque.pop_front();
        return victim;
    }
};
class OPTAlgorithm : public ReplacementAlgorithm {

    unordered_map<uint32_t, deque<int>>& futureUses;
    vector<uint32_t> currentPages;

public:
    OPTAlgorithm(unordered_map<uint32_t, deque<int>>& futures)
        : futureUses(futures) {
    }

    void updateUsage(uint32_t vpn) override {
        // Remove the current usage from the future timeline
        if (!futureUses[vpn].empty()) {
            futureUses[vpn].pop_front();
        }
        // Track that this page is now in physical memory
        if (find(currentPages.begin(), currentPages.end(), vpn) == currentPages.end()) {
            currentPages.push_back(vpn);
        }
    }

    uint32_t getVictim() override {
        uint32_t victim = 0;
        int furthestUse = -1;
        int victimIndex = -1;

        for (int i = 0; i < currentPages.size(); ++i) {
            uint32_t vpn = currentPages[i];

            // If never used again, this is the perfect victim
            if (futureUses[vpn].empty()) {
                victim = vpn;
                victimIndex = i;
                break;
            }

            // Find the page whose next use is the furthest in the future
            if (futureUses[vpn].front() > furthestUse) {
                furthestUse = futureUses[vpn].front();
                victim = vpn;
                victimIndex = i;
            }
        }

        currentPages.erase(currentPages.begin() + victimIndex);
        return victim;
    }
};