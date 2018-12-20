#include "config.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "please use like: main \"filename\"" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<shm_del_ctl> shms;
    // 创建缓冲区
    for (int i = SHM_BEG; i < SHM_END; ++i) {
        int shm = shmget(i, sizeof(Block), IPC_CREAT | 0666);
        if (shm < 0) {
            std::cerr << "create shared memory failed, errno: " << errno << std::endl;
            return EXIT_FAILURE;
        }
        shms.push_back(shm_del_ctl(new int(shm)));
    }
    // 创建信号量
    int sem = semget(SEM_ID, SEM_CNT, IPC_CREAT | 0666);
    if (sem < 0) {
        std::cerr << "create semaphores failed, errno: " << errno << std::endl;
        return EXIT_FAILURE;
    } else {
        std::cout << "create semphore success" << std::endl;
    }
    sem_del_ctl sem_del(new int(sem));
    // 初始化信号量
    semun arg;
    arg.val = 0;
    semctl(sem, VALID, SETVAL, arg);
    arg.val = BUFCNT;
    semctl(sem, EMPTY, SETVAL, arg);
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
    // 执行到return时自动调用unique_ptr的析构函数，释放控制量
    return 0;
}
