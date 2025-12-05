#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

// Function to find the running Ghost enclave and move the current thread into it
void enter_ghost() {
    int tid = syscall(SYS_gettid);
    
    // 1. Find the enclave directory in /sys/fs/ghost
    std::string enclave_path = "";
    DIR* dir = opendir("/sys/fs/ghost");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            // Look for a directory starting with "enclave_"
            if (strncmp(entry->d_name, "enclave_", 8) == 0) {
                enclave_path = std::string("/sys/fs/ghost/") + entry->d_name;
                break;
            }
        }
        closedir(dir);
    }

    if (enclave_path.empty()) {
        std::cerr << "Error: Could not find any Ghost enclave in /sys/fs/ghost. Is the Agent running?" << std::endl;
        return;
    }

    // 2. Open the 'tasks' file of the enclave
    std::string tasks_path = enclave_path + "/tasks";
    int fd = open(tasks_path.c_str(), O_WRONLY);
    if (fd < 0) {
        std::cerr << "Error: Could not open " << tasks_path << ". (Do you have root/sudo?)" << std::endl;
        return;
    }

    // 3. Write the TID to the file to join the enclave
    std::string pid_str = std::to_string(tid);
    if (write(fd, pid_str.c_str(), pid_str.length()) < 0) {
        perror("Error writing to enclave tasks file");
    } else {
        // Success!
        std::cout << "Thread " << tid << " successfully joined enclave " << enclave_path << std::endl;
    }
    close(fd);
}

void worker(int class_id) {
    pid_t tid = syscall(SYS_gettid);
    
    // 1. Move to Ghost Class BEFORE doing work
    enter_ghost(); 

    // 2. Write Class ID to /tidinfo/
    std::string filename = "/tidinfo/" + std::to_string(tid) + ".txt";
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << class_id;
        outfile.close();
        std::cout << "Thread " << tid << " registered as Class " << class_id << std::endl;
    } else {
        std::cerr << "Error: Failed to write tid info for " << tid << std::endl;
    }

    // 3. Busy Loop to generate CPU contention
    long long counter = 0;
    while (true) {
        counter++;
        volatile int x = counter * 2; 
    }
}

int main() {
    int num_classes = 4;
    std::vector<std::thread> threads;

    std::cout << "Starting ALPS workload with " << num_classes << " conflicting threads..." << std::endl;
    std::cout << "Check /tidinfo/ to confirm files are created." << std::endl;

    for (int i = 1; i <= num_classes; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}