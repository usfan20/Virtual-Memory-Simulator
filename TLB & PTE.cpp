#pragma once
#include <iostream>
#include <vector>
using namespace std;

struct PTE {
    uint32_t frameNumber = 0;  // default to frame 0
    bool valid = false;
    bool dirty = false;
};

class TLB {
    struct Entry {
        uint32_t vpn = 0;
        uint32_t frame = 0;
        bool valid = false;
    };

    vector<Entry> entries;
    uint32_t size;
    int next = 0;  // FIFO pointer — member variable, not static

public:
    TLB(uint32_t s) : size(s), next(0) {
        if (s == 0) {
            cerr << "Error: TLB size cannot be 0!" << endl;
            exit(1);
        }
        entries.resize(s);
    }

    // Returns frame number on hit, -1 on miss
    int lookup(uint32_t vpn) {
        for (auto& e : entries) {
            if (e.valid && e.vpn == vpn)
                return (int)e.frame;
        }
        return -1; // TLB Miss
    }

    // FIFO replacement for TLB entries
    void update(uint32_t vpn, uint32_t frame) {
        entries[next] = { vpn, frame, true };
        next = (next + 1) % (int)size;
    }

    // Mandatory invalidation on page eviction from RAM
    void invalidate(uint32_t vpn) {
        for (auto& e : entries) {
            if (e.valid && e.vpn == vpn)
                e.valid = false;
        }
    }

    // Flush entire TLB (useful for context switches)
    void flush() {
        for (auto& e : entries)
            e.valid = false;
        next = 0;
    }
};
