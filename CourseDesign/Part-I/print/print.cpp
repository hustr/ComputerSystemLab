#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "../config.hpp"


int main(int argc, char *argv[]) {
    std::cout << "print start" << std::endl;
    int b_empty = semget(B_EMPTY, 1, 0666);
    int b_valid = semget(B_VALID, 1, 0666);
    // get shared memory
    int b_id = shmget(SHM_B, sizeof(int), 0666);
    int *b_mem = (int *) shmat(b_id, nullptr, 0666);
    // 20 ops
    // print content in b_mem
    int *mem = new int;
    for (int i = 0; i < 20; ++i) {
        P(b_valid);
        memcpy(mem, b_mem, sizeof(int));
        std::cout << "print " << mem[0] << " from b_mem" << std::endl;
        V(b_empty);
        sleep(rand() % 6);
    }
    shmdt(b_mem);
    delete (mem);
    std::cout << "print finished" << std::endl;
    return 0;
}
