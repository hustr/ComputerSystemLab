#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <thread>

#define BSIZE 200

// 管道描述符
int pfields[2];
// 子进程pid
int cpids[2];

// 函数类型定义
typedef void(*func)(int);

int main() {
    // 创建管道
    if (pipe(pfields) != 0) {
        std::cout << "Create pipe failed\n";
        exit(EXIT_FAILURE);
    }
    // 缓冲区
    char buff[BSIZE];
    int nbytes;
    // 创建第一个子进程
    if ((cpids[0] = fork()) == 0) {
        // 子进程1
        // 关闭读管道
        close(pfields[0]);
        // 设置忽略信号
        signal(SIGINT, (func) 1);
        signal(SIGUSR1, [](int) {
            // 全局变量可以在lambda中直接访问
            close(pfields[1]);
            // 输出结束
            std::cout << "Child Process 1 is Killed by Parent!\n";
            exit(EXIT_SUCCESS);
        });
        int times = 1;
        while (true) {
            nbytes = sprintf(buff, "I send you %d times.", times);
            times++;
            // 发送数据到管道
            write(pfields[1], buff, nbytes);
            // sleep一秒
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else if ((cpids[1] = fork()) == 0) {
        // 子进程2
        close(pfields[1]);
        signal(SIGINT, (func) 1);
        signal(SIGUSR2, [](int) {
            close(pfields[0]);
            std::cout << "Child Process 2 is Killed by Parent!\n";
            exit(EXIT_SUCCESS);
        });
        while (true) {
            // 从管道读数据
            nbytes = read(pfields[0], buff, BSIZE);
            buff[nbytes] = '\0';
            std::cout << buff << "\n";
        }
    }
    close(pfields[0]);
    close(pfields[1]);
    // 主线程设置信号
    signal(SIGINT, [](int) {
        // 发送结束信号
        kill(cpids[0], SIGUSR1);
        kill(cpids[1], SIGUSR2);
    });
    // 等待结束
    waitpid(cpids[0], nullptr, 0);
    waitpid(cpids[1], nullptr, 0);
    // 结束
    std::cout << "Parent Process is Killed!\n";

    return 0;
}

