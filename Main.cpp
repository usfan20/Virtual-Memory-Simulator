#include "ReplacementAlgorithm.cpp"
#include "Config & Stats.cpp"
#include "MemoryManagement.cpp"
using namespace std;
int main() {

    SystemConfig config = ConfigParser::loadConfig("config.txt");
    StatsReport stats;
    unordered_map<uint32_t, std::deque<int>> futureMap;

    int choice = 2;
    ReplacementAlgorithm* algorithm = nullptr;

    cout << "Starting Simulation with: " << choice << endl;

    if (choice == 0) {
        algorithm = new FIFOAlgorithm();
    }
    else if (choice == 1) {

        algorithm = new LRUAlgorithm(config.getTotalFrames());
    }
    else if (choice == 2) {

        cout << "Pre-scanning trace file for OPT clairvoyance..." << endl;
        futureMap = ConfigParser::preScanTrace("trace.txt", config.Shift());
        algorithm = new OPTAlgorithm(futureMap);
    }
    else {
        cerr << "Invalid algorithm choice!" << endl;
        return 1;
    }


    MMU mmu(config, stats, algorithm);


    ifstream traceFile("trace.txt");
    string line;

    if (!traceFile.is_open()) {
        cerr << "Error: Could not open trace.txt!" << endl;
        delete algorithm;
        return 1;
    }
// read trace file
    while (getline(traceFile, line)) {
        if (line.empty()) continue;

        try {

            size_t spacePos = line.find(' ');
            uint32_t addr = stoul(line.substr(0, spacePos), nullptr, 16);
            bool isWrite = (line.substr(spacePos + 1) == "W");


            mmu.access(addr, isWrite);
        }
        catch (...) {

            continue;
        }
    }


    stats.printReport();

    delete algorithm;

    cout << "\nSimulation Complete. Press Enter to exit...";
    cin.get();

    return 0;
}