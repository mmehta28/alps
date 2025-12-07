#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <cstring>

// --- Ghost Helper Functions ---
void enter_ghost() {
    int tid = syscall(SYS_gettid);
    std::string enclave_path = "";
    DIR* dir = opendir("/sys/fs/ghost");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "enclave_", 8) == 0) {
                enclave_path = std::string("/sys/fs/ghost/") + entry->d_name;
                break;
            }
        }
        closedir(dir);
    }

    if (enclave_path.empty()) return; // Agent might not be running

    std::string tasks_path = enclave_path + "/tasks";
    int fd = open(tasks_path.c_str(), O_WRONLY);
    if (fd >= 0) {
        std::string pid_str = std::to_string(tid);
        write(fd, pid_str.c_str(), pid_str.length());
        close(fd);
    }
}

void register_class(int class_id) {
    int tid = syscall(SYS_gettid);
    std::string filename = "/tidinfo/" + std::to_string(tid) + ".txt";
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << class_id;
        outfile.close();
    }
}

// --- Fibonacci Workload ---
long long fib(int n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: simple_exp <id> <para> <class_id>" << std::endl;
        return 1;
    }

    // 1. Parse Arguments passed by start_f
    int id = std::stoi(argv[1]);
    int fib_n = std::stoi(argv[2]);
    int class_id = std::stoi(argv[3]);

    // 2. Join Ghost and Register Class
    enter_ghost();
    register_class(class_id);

    // 3. Perform CPU Intensive Work
    // printf("Task %d (Class %d) starting fib(%d)\n", id, class_id, fib_n);
    long long result = fib(fib_n);
    
    // Prevent optimization
    volatile long long keep = result; 
    
    return 0;
}