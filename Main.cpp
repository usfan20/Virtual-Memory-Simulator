#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <deque>
#include "ReplacementAlgorithm.h"
#include "Config & Stats.h"
#include "MemoryManagement.h"
using namespace std;

int main() {
    // Load configuration
    SystemConfig config = ConfigParser::loadConfig("config.txt");
    StatsReport stats;
    unordered_map<uint32_t, deque<int>> futureMap;

    // Algorithm selection with input validation
    int choice = -1;
    while (choice < 0 || choice > 2) {
        cout << "\nSelect Page Replacement Algorithm:\n";
        cout << "  0 - FIFO (First In First Out)\n";
        cout << "  1 - LRU  (Least Recently Used)\n";
        cout << "  2 - OPT  (Optimal / Clairvoyant)\n";
        cout << "Enter choice (0-2): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            choice = -1;
            cerr << "Invalid input. Please enter 0, 1, or 2.\n";
        }
        else if (choice < 0 || choice > 2) {
            cerr << "Invalid choice. Please enter 0, 1, or 2.\n";
        }
    }
    cin.ignore(1000, '\n'); // clear leftover newline

    // Create algorithm
    ReplacementAlgorithm* algorithm = nullptr;
    if (choice == 0) {
        cout << "\n>> FIFO PAGE REPLACEMENT ALGORITHM\n";
        algorithm = new FIFOAlgorithm();
    }
    else if (choice == 1) {
        cout << "\n>> LRU PAGE REPLACEMENT ALGORITHM\n";
        algorithm = new LRUAlgorithm(config.getTotalFrames());
    }
    else {
        cout << "\n>> OPT PAGE REPLACEMENT ALGORITHM\n";
        cout << "Pre-scanning trace file for OPT clairvoyance...\n";
        futureMap = ConfigParser::preScanTrace("trace.txt", config.Shift());
        algorithm = new OPTAlgorithm(futureMap);
    }

    // Open trace file
    ifstream traceFile("trace.txt");
    if (!traceFile.is_open()) {
        cerr << "Error: Could not open trace.txt!" << endl;
        delete algorithm;
        return 1;
    }

    // Create MMU
    MMU mmu(config, stats, algorithm);

    // Process trace
    string line;
    long long lineNumber = 0;
    long long skipped = 0;

    while (getline(traceFile, line)) {
        lineNumber++;
        if (line.empty()) continue;

        // Trim carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        try {
            size_t spacePos = line.find(' ');
            if (spacePos == string::npos) {
                cerr << "Warning: Line " << lineNumber
                    << " malformed (no space found), skipping.\n";
                skipped++;
                continue;
            }

            uint32_t addr = stoul(line.substr(0, spacePos), nullptr, 16);
            string op = line.substr(spacePos + 1);

            // Trim op whitespace
            op.erase(op.find_last_not_of(" \t\r\n") + 1);

            // Out of bounds check — 32-bit address space
            if (addr > 0xFFFFFFFF) {
                cerr << "Warning: Line " << lineNumber
                    << " address out of 32-bit bounds, skipping.\n";
                skipped++;
                continue;
            }

            bool isWrite = (op == "W");
            mmu.access(addr, isWrite);
        }
        catch (...) {
            cerr << "Warning: Line " << lineNumber
                << " could not be parsed, skipping.\n";
            skipped++;
        }
    }

    traceFile.close();

    cout << "\nTrace complete. Total lines: " << lineNumber
        << ", Skipped: " << skipped << "\n";

    stats.printReport();

    delete algorithm;

    cout << "\nSimulation Completed.\n";
    cout << "Press Enter to exit...";
    cin.get();

    return 0;
}
