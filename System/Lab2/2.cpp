#include <iostream> // cout cerr
#include <sys/sem.h> // semget semctl semop
#include <pthread.h> // thread: create, join
#include <errno.h> // errno
#include <chrono> // milliseconds
#include <thread> // yield, sleep

#define KEY 101 /* 信号量的key */
#define SEM_NUM 1 /* 信号量数量 */
#define END 100 /* 售卖票的数量 */
#define WINS 10 /* 窗口数量 */

// 记录已销售的
volatile int selled = 0;
int sem_id = 0;
// 此数据结构需要用户自己显式定义
/* arg for semctl system calls. */
union semun {
    int val;            /* value for SETVAL */
    semid_ds *buf;    /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;    /* array for GETALL & SETALL */
};

void P(int semid, int index = 0) {
    sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
}

void V(int semid, int index = 0) {
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
    int ids[WINS];
    for (int i = 0; i < WINS; ++i) {
        // 编号
        ids[i] = i;
        // 创建线程
        pthread_create(pid_sell + i, nullptr, [](void *num) -> void * {
            int sum = 0; // 此线程售票数
            while (true) {
                P(sem_id);
                // 已销售0，此线程应该销售1
                if (selled < END) {
                    ++sum;
                    ++selled;
                    std::cout << *static_cast<int*>(num) << " sells " << selled << "\n";
                    V(sem_id);
                    std::this_thread::sleep_for(std::chrono::milliseconds(random() % 100));
                } else {
                    // 输出本窗口所有票数
                    std::cout << "thread " << *static_cast<int*>(num) 
                    << " selled " << sum << " ticket(s) in total.\n";
                    V(sem_id);
                    break;
                }
            }
            // 线程结束
            return nullptr;
        }, ids + i);
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

