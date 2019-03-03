#include "../config.hpp"

int main(int argc, char *argv[]) {
    std::cout << "read start" << std::endl;
    int a_empty = semget(A_EMPTY, 1, 0666);
    int a_valid = semget(A_VALID, 1, 0666);
    // get shared memory
    int a_id = shmget(SHM_A, sizeof(int), 0666);
    int *a_mem = (int *) shmat(a_id, nullptr, 0666);
    // 20 ops
    for (int i = 0; i < 20; ++i) {
        P(a_empty);
        a_mem[0] = i;
        std::cout << "read " << a_mem[0] << " to a_mem" << std::endl;
        V(a_valid);
        sleep(rand() % 4);
    }
    shmdt(a_mem);
    std::cout << "read finished" << std::endl;
    return 0;
}
