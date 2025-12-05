#include <iostream>
#include <unistd.h>

int main() {
    // Infinite loop to burn 100% CPU
    while (true) {
        volatile int x = 1 + 1;
    }
    return 0;
}
