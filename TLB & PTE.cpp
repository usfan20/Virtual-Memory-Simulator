#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include<list>
#include<unordered_map>
#include<deque>
using namespace std;
struct PTE {
    uint32_t frameNumber;
    bool valid = false;
    bool dirty = false;
};

class TLB {
    struct Entry { uint32_t vpn; uint32_t frame; bool valid = false; };
    vector<Entry> entries;
    uint32_t size;

public:
    TLB(uint32_t s) : size(s) { entries.resize(s); }

    int lookup(uint32_t vpn) {
        for (auto& e : entries) {
            if (e.valid && e.vpn == vpn) return e.frame;
        }
        return -1; // TLB Miss
    }

    void update(uint32_t vpn, uint32_t frame) {
        // Simple FIFO update for TLB entries
        static int next = 0;
        entries[next] = { vpn, frame, true };
        next = (next + 1) % size;
    }

    void invalidate(uint32_t vpn) { // Mandatory for RAM eviction
        for (auto& e : entries) {
            if (e.vpn == vpn) e.valid = false;
        }
    }
};