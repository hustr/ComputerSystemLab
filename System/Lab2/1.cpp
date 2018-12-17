#include <iostream>
#include <sys/sem.h>
#include <pthread.h>
#include <errno.h>

#define KEY 100
#define SEM_NUM 2
#define END 100
#define VALID 0
#define EMPTY 1

volatile int a = 0;
int sem_id = 0;
// 此数据结构需要用户自己显式定义
/* arg for semctl system calls. */
union semun {
    int val;            /* value for SETVAL */
    semid_ds *buf;    /* buffer for IPC_STAT & IPC_SET */
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
    // valid初始值为0
    arg.val = 0;
    if (semctl(sem_id, VALID, SETVAL, arg) < 0) {
        std::cerr << "set value failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
    // empty初始值为1
    arg.val = 1;
    if (semctl(sem_id, EMPTY, SETVAL, arg) < 0) {
        std::cerr << "set value failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }

    pthread_t pid_compute, pid_print;
    // 创建计算线程
    pthread_create(&pid_compute, nullptr, [](void *) -> void * {
        while (true) {
            // 请求空缓冲区
            P(sem_id, EMPTY);
            if (a < END) {
                ++a;
                // 标记数据为有效
                V(sem_id, VALID);
            } else {
                // 数据有效还是无效呢？
                V(sem_id, VALID);
                break;
            }
        }
        return nullptr;
    }, nullptr);
    if (pid_compute < 0) {
        std::cerr << "create thread failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
    // 创建打印线程
    pthread_create(&pid_print, nullptr, [](void *) -> void * {
        while (true) {
            // 请求有效数据
            P(sem_id, VALID);
            if (a <= END) {
                std::cout << "print a: " << a << "\n";
                // 给出空缓冲区
                V(sem_id, EMPTY);
            } else {
                // 虽然没啥意义，但还是释放一下吧
                V(sem_id, EMPTY);
                break;
            }
        }
        return nullptr;
    }, nullptr);
    if (pid_print < 0) {
        std::cerr << "create thread failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }
    //  阻塞主线程
    pthread_join(pid_compute, nullptr);
    pthread_join(pid_print, nullptr);
    // 销毁信号量,第二个参数0在此处无意义
    if (semctl(sem_id, 1024, IPC_RMID) < 0) {
        std::cerr << "destroy sem failed, errno: " << errno << "\n";
        exit(EXIT_FAILURE);
    }

    return 0;
}
