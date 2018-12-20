#include "config.hpp"

int main(int argc, char *argv[]) {
    // put直接打开原文件
     int fd = open(argv[0], O_RDONLY);
     if (fd < 0) {
         std::cerr << "open " << argv[0] << "failed, errno: " << errno << std::endl;
         return EXIT_FAILURE;
     }
    // 获取虚拟内存块
    Block *blocks[SHM_END - SHM_BEG];
    std::vector<shm_dt_ctl> del_shms;
    for (int i = SHM_BEG; i < SHM_END; ++i) {
        int shm = shmget(i, sizeof(Block), 0666);
        blocks[i - SHM_BEG] = static_cast<Block*>(shmat(shm, nullptr, 0));
        del_shms.push_back(shm_dt_ctl(blocks[i - SHM_BEG]));
    }
    // 获取信号灯
    int sem = semget(SEM_ID, SEM_CNT, 0666);
    std::cout << "put sem: " << sem << std::endl;
    // 写入文件
    int block_idx = 0;
    char buf[BUFSIZE];
    while (true) {
        auto &blk = *blocks[block_idx];
        // 写入文件
        P(sem, EMPTY);
        int bytes = read(fd, blk.data, BUFSIZE);
        blk.size = bytes;
        // 结束判断
        blk.end = bytes < BUFSIZE;
        V(sem, VALID);
        std::cout << "put: " << block_idx << std::endl;
        block_idx = (block_idx + 1) % BUFCNT;
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
