#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <utility>
#include <cerrno>
#include <cassert>
#include <vector>
#include <thread>
#include <chrono>

// 缓冲区个数100个
#define BUFCNT 100
//  一个缓冲区10字节
#define BUFSIZE 10

// 缓冲区ID
const key_t SHM_BEG = 100;
const key_t SHM_END = 200;
// 信号量ID
const key_t SEM_ID = 101;
const key_t VALID = 0;
const key_t EMPTY = 1;
const size_t SEM_CNT = 2;


struct DEL_SHM {
    void operator()(int *id) {
        shmctl(*id, IPC_RMID, nullptr);
        delete id;
    }
};

typedef std::unique_ptr<int, DEL_SHM> shm_del_ctl;

struct Block;

struct DT_SHM {
    void operator()(Block *addr) {
        shmdt(addr);
    }
};

typedef std::unique_ptr<Block, DT_SHM> shm_dt_ctl;

struct DEL_SEM {
    void operator()(int *id) {
        semctl(*id, 0, IPC_RMID);
        delete id;
    }
};

typedef std::unique_ptr<int, DEL_SEM> sem_del_ctl;

void P(int semid, int index) {
    sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
}

void V(int semid, int index) {
    sembuf sem;
    sem.sem_num = index;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
}

struct Block {
    // 文件块定义
    bool end{}; // 标志文件结束
    int size{}; // 二进制数据数目
    char data[BUFSIZE];// 数据部分
};

union semun {
    int val;
    semid_ds *buf;
    unsigned short *array;
};

#endif // __CONFIG_H__
