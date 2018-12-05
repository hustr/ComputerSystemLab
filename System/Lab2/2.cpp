#include <iostream> // cout cerr
#include <sys/sem.h> // semget semctl semop
#include <pthread.h> // thread: create, join
#include <errno.h> // errno
#include <chrono> // milliseconds
#include <memory> // unique_ptr
#include <utility> // make_unique
#include <thread> // yield, sleep

#define KEY 101
#define SEM_NUM 1
#define END 100
#define WINS 10

// 记录已销售的
volatile int selled = 0;
int sem_id = 0;
// 此数据结构需要用户自己显式定义
/* arg for semctl system calls. */
union semun {
    int val;            /* value for SETVAL */
    struct semid_ds *buf;    /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;    /* array for GETALL & SETALL */
};

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

int main() {
    sem_id = semget(KEY, SEM_NUM, IPC_CREAT | 0666);
    if (sem_id < 0) {
        std::cerr << "Create sem failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
    // 设置信号量的初始值
    semun arg{};
    // 变量互斥类型的，初值为1
    arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, arg) < 0) {
        std::cerr << "set value failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
    // 销售窗口pid
    pthread_t pid_sell[WINS];
    // 创建售票线程
    for (int i = 0; i < WINS; ++i) {
        // 编号
        std::unique_ptr<int> num = std::make_unique<int>(i);
        // 创建线程
        pthread_create(pid_sell + i, nullptr, [](void *num) -> void * {
            // 移动构造pid
            auto pid = std::move(*static_cast<std::unique_ptr<int> *>(num));
            while (true) {
                // 请求有效数据
                P(sem_id, 0);
                // 已销售0，此线程应该销售1
                if (selled < END) {
                    ++selled;
                    std::cout << *pid << " sells " << selled << "\n";
                    V(sem_id, 0);
                    // 随机睡眠
                    std::this_thread::sleep_for(std::chrono::milliseconds(random() % 100));
                } else {
                    // 结束循环
                    V(sem_id, 0);
                    break;
                }
            }
            // 线程结束
            return nullptr;
        }, &num);
        //  等待线程将num指向的内存取走
        while (num) {
            std::this_thread::yield();
        }
        if (pid_sell[i] < 0) {
            std::cerr << "create thread failed, errno: " << errno << "\n";
            exit(EXIT_FAILURE);
        }
    }

    //  阻塞主线程
    for (auto &pid : pid_sell) {
        pthread_join(pid, nullptr);
    }
    // 销毁信号量,第二个参数0在此处无意义
    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        std::cerr << "destroy sem failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }

    return 0;
}

