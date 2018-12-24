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


//  一个缓冲区10字节
#define BUFSIZE 10
// 缓冲区ID
const key_t SHM_BEG = 100;
const key_t SHM_END = 200;
// 缓冲区个数
#define BUFCNT (SHM_END - SHM_BEG)
// 信号量ID
const key_t SEM_VALID = 1000;
const key_t SEM_EMPTY = 1001;

void P(int semid, int index = 0) {
    sembuf sem{};
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
}

void V(int semid, int index = 0) {
    sembuf sem{};
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
// 信号量控制参数
union semun {
    int val;
    semid_ds *buf;
    unsigned short *array;
};

// 代码重构，将虚拟共享内存的attach和detach交给类来处理，内存的创建和销毁交给类
// 信号量的创建和释放交给类控制
// 内存创建与销毁
template<typename T>
class ShmBlk {
private:
    // 申请到的内存的id
    int id = -1;
    pid_t pid = -1;
public:
    ShmBlk() = default;
    // 禁止拷贝和赋值
    ShmBlk(const ShmBlk&) = delete;
    ShmBlk& operator=(const ShmBlk&) = delete;

    ShmBlk(key_t key) : pid(getpid()) {
        // 申请内存
        id = shmget(key, sizeof(T), IPC_CREAT | 0666);
        if (id < 0) {
            std::cerr << "create shared memory failed, errno: " << errno << std::endl;
            return;
        }
    }

    // emplace_back是个坑啊
    ShmBlk(ShmBlk &&other) noexcept {
        id = other.id;
        pid = other.pid;
        other.id = -1;
        other.pid = -1;
    }

    ~ShmBlk() {
        if (id != -1 && getpid() == pid) {
            int del = shmctl(id, IPC_RMID, nullptr);
            if (del < 0) {
                // 内存删除失败
                std::cerr << "delete shared memory failed, errno: " << errno << std::endl;
            }
        }
    }
};

// 内存的attach和detach
template<typename T>
class ShmCtl {
private:
    T *addr = nullptr;
public:
    ShmCtl() = default;
    ShmCtl(const ShmCtl&) = delete;
    ShmCtl& operator=(const ShmCtl&) = delete;

    // 通过id来attach内存
    explicit ShmCtl(int id) {
        // 返回值是内存地址,失败返回-1
        addr = static_cast<Block*>(shmat(id, nullptr, 0666));
        if (size_t(addr) == -1) {
            std::cerr << "attach shared memory failed, errno: " << errno << std::endl;
        }
    }

    // 坑啊。。。
    ShmCtl(ShmCtl &&other) noexcept {
        addr = other.addr;
        other.addr = nullptr;
    }

    T &get() {
        return *addr;
    }

    ~ShmCtl() {
        if (addr != nullptr) {
            int dt = shmdt(addr);
            if (dt < 0) {
                std::cerr << "detach shared memory failed, errno: " << errno << std::endl;
            }
        }
    }
};

// 信号量的创建与销毁
class SemBlk {
private:
    int id = -1;
    pid_t pid = -1;
public:
    SemBlk() = delete;
    SemBlk(const SemBlk&) = delete;
    SemBlk &operator=(const SemBlk&) = delete;

    explicit SemBlk(key_t key) : pid(getpid()) {
        id = semget(key, 1, IPC_CREAT | 0666);
        if (id < 0) {
            std::cerr << "create semaphore failed, errno: " << errno << std::endl;
        }
    }

    SemBlk(SemBlk &&other) noexcept {
        id = other.id;
        pid = other.pid;
        other.id = -1;
        other.pid = -1;
    }

    explicit operator int() {
        return id;
    }

    ~SemBlk() {
        if (id != -1 && pid == getpid()) {
            int del = semctl(id, 0, IPC_RMID);
            if (del < 0) {
                std::cerr << "delete semaphore failed, errno: " << errno << std::endl;
            }
        }
    }
};

#endif // __CONFIG_H__
