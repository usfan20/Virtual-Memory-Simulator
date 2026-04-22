# Virtual-Memory-Simulator
A high-fidelity simulation of a 32-bit Virtual Memory System, designed to analyze the performance of various page replacement policies. This project models the interaction between a Memory Management Unit (MMU), Translation Lookaside Buffer (TLB), and Physical RAM to calculate system efficiency and access latencies. It is developed my Usman Irfan and Mobeen Zafar for OS Project. 


## Key Features

* **32-Bit Architecture Simulation**: Handles a full 32-bit address space.
* **Two-Level Lookup**: Implements both TLB (fast cache) and Page Table (RAM) translation paths.
* **Dirty Bit Logic**: Simulates realistic disk write-back penalties (10ms) for modified pages.
* **Flexible Hardware Config**: Easily adjust RAM size, Page size, and TLB size via an external configuration file.
* **Pre-Scan Optimization**: Includes a "clairvoyant" pre-scan of memory traces to support the Optimal (OPT) replacement algorithm.

1.  **FIFO (First-In-First-Out)**: Replaces the oldest page in memory. Used to demonstrate **Belady's Anomaly**.
2.  **LRU (Least Recently Used)**: Replaces the page that hasn't been accessed for the longest period.
3.  **OPT (Optimal)**: Replaces the page that will not be used for the longest period in the future. This provides the theoretical minimum fault rate for comparison.

## Configuration

The system is configured via `config.txt`. and `trace.txt` are uploaded with project you may edit the values inside.Both files are required to have valid values to run the program successfully
Visual Studio 2022 was used for compiling and running the project.
