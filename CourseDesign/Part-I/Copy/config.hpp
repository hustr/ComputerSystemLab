#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>

// semphore
#define A_EMPTY 100
#define A_VALID 101
#define B_EMPTY 102
#define B_VALID 103

#define SHM_A 104
#define SHM_B 105
#define BUF_SIZE 20

struct Block {
    bool end;
    int len;
    char data[BUF_SIZE];
};

union semun {
    int val;
    semid_ds *buf;
    unsigned short *array;
};

void P(int id, unsigned short idx = 0);

void V(int id, unsigned short idx = 0);

#endif // CONFIG_HPP
