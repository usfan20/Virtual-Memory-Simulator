#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include<list>
#include<unordered_map>
#include<deque>
using namespace std;
struct SystemConfig {
    uint32_t ramSize;     // e.g., 256KB to 2GB 
    uint32_t pageSize;    // e.g., 1KB to 64KB
    uint32_t tlbSize;     // e.g., 4 to 512 

    double tlbLatency = 1.0;
    double ramLatency = 100.0;
    double diskLatency = 10000000.0;

    uint32_t Shift() const { return log2(pageSize); }
    uint32_t Mask() const { return pageSize - 1; }
    uint32_t getTotalFrames() const { return ramSize / pageSize; }
};
class ConfigParser {
public:
    static SystemConfig loadConfig(const string& filename) {
        SystemConfig config;
        ifstream file(filename);
        string line;

        if (!file.is_open()) {
            cerr << "Error: Could not open config file!" << endl;
            return config;
        }

        while (getline(file, line)) {
            // Skip comments or empty lines
            if (line.empty() || line[0] == '#') continue;

            istringstream is_line(line);
            string key;

            // Split at the colon
            if (getline(is_line, key, ':')) {
                string value;
                if (getline(is_line, value)) {
                    if (key == "RAM_SIZE") config.ramSize = stoul(value);
                    else if (key == "PAGE_SIZE") config.pageSize = stoul(value);
                    else if (key == "TLB_SIZE") config.tlbSize = stoul(value);
                    else if (key == "TLB_LATENCY") config.tlbLatency = stod(value);
                    else if (key == "RAM_LATENCY") config.ramLatency = stod(value);
                    else if (key == "DISK_LATENCY") config.diskLatency = stod(value);
                }
            }
        }
        return config;
    }

    static unordered_map<uint32_t, std::deque<int>> preScanTrace(std::string filename, uint32_t shift) {
        unordered_map<uint32_t, std::deque<int>> futureUses;
        ifstream file(filename);
        string line;
        int lineCount = 0;

        if (!file.is_open()) {
            std::cerr << "Error: Could not open trace file for pre-scan!" << std::endl;
            return futureUses;
        }

        while (std::getline(file, line)) {
            if (line.empty()) continue;


            uint32_t addr = std::stoul(line.substr(0, line.find(' ')), nullptr, 16);
            uint32_t vpn = addr >> shift;

            futureUses[vpn].push_back(lineCount);
            lineCount++;


        }
        return futureUses;
    }
};
class StatsReport {
public:
    long totalAccesses = 0;
    long tlbHits = 0, ptHits = 0, pageFaults = 0, diskWrites = 0;
    long long totalTimeNs = 0;

    void printReport() {
        std::cout << "\n    Performance Report    \n";
        std::cout << "Total Accesses: " << totalAccesses << "\n";
        std::cout << "TLB Hit Ratio: " << (double)tlbHits / totalAccesses * 100 << "%\n";
        std::cout << "Page Faults: " << pageFaults << " (Rate: " << (double)pageFaults / totalAccesses * 100 << "%)\n";
        std::cout << "Disk Writes: " << diskWrites << "\n";
        std::cout << "Simulated Time: " << totalTimeNs << " ns\n";
        std::cout << "EAT: " << totalTimeNs / totalAccesses << " ns\n";
    }



};