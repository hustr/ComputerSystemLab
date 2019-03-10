#include "config.hpp"
void P(int id, unsigned short idx) {
    sembuf sem{};
    sem.sem_num = idx;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(id, &sem, 1);
}

void V(int id, unsigned short idx) {
    sembuf sem{};
    sem.sem_num = idx;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(id, &sem, 1);
}
