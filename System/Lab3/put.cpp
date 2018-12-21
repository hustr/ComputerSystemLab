#include "config.hpp"

int main(int argc, char *argv[]) {
    // put直接打开原文件
     int fd = open(argv[0], O_RDONLY);
     if (fd < 0) {
         std::cerr << "open " << argv[0] << "failed, errno: " << errno << std::endl;
         return EXIT_FAILURE;
     }
    // 获取虚拟内存块
    std::vector<ShmCtl> shms;
    for (int i = SHM_BEG; i < SHM_END; ++i) {
        int shm = shmget(i, sizeof(Block), 0666);
        shms.emplace_back(shm);
    }
    // 获取信号灯
    int valid = semget(SEM_VALID, 1, 0666);
    int empty = semget(SEM_EMPTY, 1, 0666);
    // 写入文件
    int idx = 0;
    char buf[BUFSIZE];
    while (true) {
        auto &blk = shms[idx].get<Block>();
        // 写入文件
        P(empty);
        int bytes = read(fd, blk.data, BUFSIZE);
        blk.size = bytes;
        // 结束判断
        blk.end = bytes < BUFSIZE;
        V(valid);
        std::cout << "put: " << idx << std::endl;
        idx = (idx + 1) % BUFCNT;
        // 结束就停止读取
        if (blk.end) {
            break;
        }
    }
    std::cout << "put end" << std::endl;
    // 关闭文件描述符
    close(fd);
    // 结束写入程序
    return 0;
}
