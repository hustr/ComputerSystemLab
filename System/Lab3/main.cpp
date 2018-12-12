#include "config.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "please use like: main \"filename\"" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 创建缓冲区
    int shm = shmget(SHM_ID, BUFCNT * sizeof(Block), IPC_CREAT | 0666);
    if (shm < 0) {
        std::cerr << "create shared memory failed, errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "create shared memory success" << std::endl;
    }
    // 创建信号量
    int sem = semget(SEM_ID, SEM_CNT, IPC_CREAT | 0666);
    if (sem < 0) {
        std::cerr << "create semaphores failed, errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "create semphore success" << std::endl;
    }
    // 初始化信号量
    semun arg;
    arg.val = 0;
    std::cout << semctl(sem, VALID, SETVAL, arg) << std::endl;
    arg.val = BUFCNT;
    std::cout << semctl(sem, EMPTY, SETVAL, arg) << std::endl;
    // 创建两个子进程
    pid_t pids[2];
    // 传一个参数即文件名即可
    char *const sub_argv[] = {argv[1], nullptr};
    if ((pids[0] = fork()) == 0) {
        // 切换到发送程序
        execv("put", sub_argv);
    } else if ((pids[1] = fork()) == 0) {
        // 切换到接收程序
        execv("get", sub_argv);
    }
    // 等待结束
    waitpid(pids[0], nullptr, 0);
    waitpid(pids[1], nullptr, 0);
    if(shmctl(shm, 0, IPC_RMID) < 0) {
        std::cerr << "destroy shared memory failed, errno: " << errno << std::endl;
    } else {
        std::cout << "destroy shared memory success" << std::endl;
    }
    if (semctl(sem, 0, IPC_RMID) < 0) {
        std::cerr << "destroy semphore failed, errno: " << errno << std::endl;
    } else {
        std::cout << "destroy semphore success" << std::endl;
    }

    // 执行到return时自动调用unique_ptr的析构函数，释放控制量
    return 0;
}
