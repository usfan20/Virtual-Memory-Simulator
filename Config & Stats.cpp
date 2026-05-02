#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <list>
#include <unordered_map>
#include <deque>
#include <cmath>
using namespace std;

struct SystemConfig {
    uint32_t ramSize = 524288;   // default 512KB
    uint32_t pageSize = 4096;     // default 4KB
    uint32_t tlbSize = 16;       // default 16 entries

    double tlbLatency = 1.0;
    double ramLatency = 100.0;
    double diskLatency = 1000.0;

    uint32_t Shift() const {
        return (uint32_t)log2((double)pageSize);
    }
    uint32_t Mask() const {
        return pageSize - 1;
    }
    uint32_t getTotalFrames() const {
        return ramSize / pageSize;
    }

    // Validate config values after loading
    bool validate() const {
        if (ramSize == 0) {
            cerr << "Error: RAM_SIZE cannot be 0!" << endl;
            return false;
        }
        if (pageSize == 0) {
            cerr << "Error: PAGE_SIZE cannot be 0!" << endl;
            return false;
        }
        if (tlbSize == 0) {
            cerr << "Error: TLB_SIZE cannot be 0!" << endl;
            return false;
        }
        if (ramSize < pageSize) {
            cerr << "Error: RAM_SIZE must be >= PAGE_SIZE!" << endl;
            return false;
        }
        // Check page size is power of 2
        if ((pageSize & (pageSize - 1)) != 0) {
            cerr << "Error: PAGE_SIZE must be a power of 2!" << endl;
            return false;
        }
        return true;
    }
};

class ConfigParser {
public:
    static SystemConfig loadConfig(const string& filename) {
        SystemConfig config;
        ifstream file(filename);
        string line;

        if (!file.is_open()) {
            cerr << "Error: Could not open config file: " << filename << endl;
            cerr << "Using default configuration." << endl;
            return config;
        }

        while (getline(file, line)) {
            // Skip comments or empty lines
            if (line.empty() || line[0] == '#') continue;

            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            istringstream is_line(line);
            string key;

            if (getline(is_line, key, ':')) {
                string value;
                if (getline(is_line, value)) {
                    // Trim value whitespace
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);

                    try {
                        if (key == "RAM_SIZE")    config.ramSize = stoul(value);
                        else if (key == "PAGE_SIZE")   config.pageSize = stoul(value);
                        else if (key == "TLB_SIZE")    config.tlbSize = stoul(value);
                        else if (key == "TLB_LATENCY") config.tlbLatency = stod(value);
                        else if (key == "RAM_LATENCY") config.ramLatency = stod(value);
                        else if (key == "DISK_LATENCY")config.diskLatency = stod(value);
                    }
                    catch (...) {
                        cerr << "Warning: Could not parse config value for key: " << key << endl;
                    }
                }
            }
        }

        // Validate after loading
        if (!config.validate()) {
            cerr << "Error: Invalid configuration. Exiting." << endl;
            exit(1);
        }

        cout << "Config loaded: RAM=" << config.ramSize
            << "B, PageSize=" << config.pageSize
            << "B, TLBSize=" << config.tlbSize
            << ", Frames=" << config.getTotalFrames() << endl;

        return config;
    }

    static unordered_map<uint32_t, deque<int>> preScanTrace(
        const string& filename, uint32_t shift)
    {
        unordered_map<uint32_t, deque<int>> futureUses;
        ifstream file(filename);
        string line;
        int lineCount = 0;

        if (!file.is_open()) {
            cerr << "Error: Could not open trace file for pre-scan: " << filename << endl;
            return futureUses;
        }

        while (getline(file, line)) {
            if (line.empty()) continue;
            try {
                size_t spacePos = line.find(' ');
                if (spacePos == string::npos) continue;
                uint32_t addr = stoul(line.substr(0, spacePos), nullptr, 16);
                uint32_t vpn = addr >> shift;
                futureUses[vpn].push_back(lineCount);
                lineCount++;
            }
            catch (...) {
                cerr << "Warning: Skipping malformed line in pre-scan: " << line << endl;
            }
        }

        cout << "Pre-scan complete. Unique VPNs found: " << futureUses.size() << endl;
        return futureUses;
    }
};

class StatsReport {
public:
    long totalAccesses = 0;
    long tlbHits = 0;
    long ptHits = 0;
    long pageFaults = 0;
    long diskWrites = 0;
    long long totalTimeNs = 0;

    void printReport() {
      
        cout << "  ------  Performance Report  ------   \n";
       

        if (totalAccesses == 0) {
            cout << "No accesses recorded.\n";
            return;
        }

        double tlbHitRatio = (double)tlbHits / totalAccesses * 100.0;
        double ptHitRatio = (double)ptHits / totalAccesses * 100.0;
        double faultRate = (double)pageFaults / totalAccesses * 100.0;
        double eat = (double)totalTimeNs / totalAccesses;

        cout << "Total Accesses   : " << totalAccesses << "\n";
        cout << "TLB Hits         : " << tlbHits
            << " (Hit Ratio: " << tlbHitRatio << "%)\n";
        cout << "Page Table Hits  : " << ptHits
            << " (Hit Ratio: " << ptHitRatio << "%)\n";
        cout << "Page Faults      : " << pageFaults
            << " (Fault Rate: " << faultRate << "%)\n";
        cout << "Disk Writes      : " << diskWrites << "\n";
        cout << "Simulated Time   : " << totalTimeNs << " ns\n";
        cout << "EAT              : " << eat << " ns\n";
        cout << "==============================\n";
    }
};
