#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

// A CPU-heavy task to simulate work
long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./fib_task <target_duration_ms>" << std::endl;
        return 1;
    }

    int target_ms = std::atoi(argv[1]);
    auto start = std::chrono::high_resolution_clock::now();
    
    // Spin the CPU until the target duration is met
    // (This prevents the task from sleeping, forcing the scheduler to manage it)
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed >= target_ms) break;
        
        // Run a small calculation chunk
        volatile long long x = fib(25); 
    }

    return 0;
}
