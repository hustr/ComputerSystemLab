#include "config.hpp"

int main(int argc, char *argv[]) {
    std::string name = argv[0];
    name += ".cpy";
    struct stat statbuf;
    stat(argv[0], &statbuf);
    // get创建一个copy文件
    int fd = open(name.c_str(), O_WRONLY | O_CREAT, statbuf.st_mode);
    if (fd < 0 ) {
        std::cerr << "open " << argv[0] << " failed, errno: " << errno << std::endl;
        return EXIT_FAILURE;
    }
    Block *blocks[SHM_END - SHM_BEG];
    std::vector<std::unique_ptr<Block, DT_SHM>> dt_shms;
    for (int i = SHM_BEG; i < SHM_END; ++i) {
        int shm = shmget(i, sizeof(Block), 0666);
        blocks[i - SHM_BEG] = static_cast<Block*>(shmat(shm, nullptr, 0));
        dt_shms.push_back(std::unique_ptr<Block, DT_SHM>(blocks[i - SHM_BEG]));
    }
    // 获取信号灯
    int sem = semget(SEM_ID, SEM_CNT, 0666);
    std::cout << "get sem: " << sem << std::endl;
    int block_idx = 0;
    while (true) {
        // 写入文件
        std::cout << "get: " << block_idx << std::endl;
        auto &blk = *blocks[block_idx];
        P(sem, VALID);
        // file.write(blk.data, blk.size);
        write(fd, blk.data, blk.size);
        V(sem, EMPTY);
        // 判断是否结束
        if (blk.end) {
            break;
        }
        block_idx = (block_idx + 1) % BUFCNT;
    }
    std::cout << "get end" << std::endl;
    return 0;
}
