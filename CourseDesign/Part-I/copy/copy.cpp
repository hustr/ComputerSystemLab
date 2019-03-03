#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "../config.hpp"


int main(int argc, char *argv[]) {
    std::cout << "copy start" << std::endl;
    int a_empty = semget(A_EMPTY, 1, 0666);
    int a_valid = semget(A_VALID, 1, 0666);
    int b_empty = semget(B_EMPTY, 1, 0666);
    int b_valid = semget(B_VALID, 1, 0666);
    // get shared memory
    int a_id = shmget(SHM_A, sizeof(int), 0666);
    int b_id = shmget(SHM_B, sizeof(int), 0666);

    int *a_mem = (int *) shmat(a_id, nullptr, 0666);
    int *b_mem = (int *) shmat(b_id, nullptr, 0666);
    int *my_mem = new int;
    // 20 ops
    // copy content from a_mem to my_mem, then copy content from my_mem to b_mem
    for (int i = 0; i < 20; ++i) {
        P(a_valid);
        memcpy(my_mem, a_mem, sizeof(int));
        std::cout << "copy " << my_mem[0] << " to my_mem" << std::endl;
        V(a_empty);
        P(b_empty);
        memcpy(b_mem, my_mem, sizeof(int));
        std::cout << "copy " << my_mem[0] << " to b_mem" << std::endl;
        V(b_valid);
        sleep(rand() % 4);
    }
    shmdt(a_mem);
    shmdt(b_mem);
    delete (my_mem);
    std::cout << "copy finished" << std::endl;
    return 0;
}
