#include "config.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "please use like: main \"filename\"" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<ShmBlk> shms;
    // 创建缓冲区
    for (key_t i = SHM_BEG; i < SHM_END; ++i) {
        shms.emplace_back(i, sizeof(Block));
    }
    // 创建信号量
    SemBlk valid(SEM_VALID), empty(SEM_EMPTY);
    // 初始化信号量
    semun arg;
    arg.val = 0;
    semctl(int(valid), 0, SETVAL, arg);
    arg.val = BUFCNT;
    semctl(int(empty), 0, SETVAL, arg);
    // 创建两个子进程
    pid_t pids[2];
    // 传一个参数即文件名即可
    char *const sub_argv[] = {argv[1], nullptr};
    std::cout << "fork from here\n";
    std::cout << "size: " << shms.size() << "\n";
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
