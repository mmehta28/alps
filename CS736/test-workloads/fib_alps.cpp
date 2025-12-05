#include <iostream>
#include <vector>
#include <chrono>
#include <unistd.h> // for getpid
#include <cstdlib>

long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./fib_alps <fib_number>" << std::endl;
        return 1;
    }
    int n = std::atoi(argv[1]);

    // 1. Print PID immediately (Needed for /tidinfo/<PID>.txt)
    std::cout << "PID:" << getpid() << std::endl;
    
    // 2. Allow a tiny window for the Python script to write the policy file
    // and for the agent to move this task to ghOSt.
    usleep(10000); // 10ms wait

    // 3. Measure Execution Time
    auto start = std::chrono::high_resolution_clock::now();
    long long res = fib(n);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // 4. Output results
    std::cout << "[Result] Fib(" << n << ") Time: " << duration << " ms" << std::endl;
    return 0;
}
