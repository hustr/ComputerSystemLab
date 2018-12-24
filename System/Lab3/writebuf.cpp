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
    std::vector<ShmCtl<Block>> shms;
    for (int i = SHM_BEG; i < SHM_END; ++i) {
        int shm = shmget(i, sizeof(Block), 0666);
        shms.emplace_back(shm);
    }
    // 获取信号灯
    int valid = semget(SEM_VALID, 1, 0666);
    int empty = semget(SEM_EMPTY, 1, 0666);
    int idx = 0;
    while (true) {
        // 写入文件
        auto &blk = shms[idx].get();
        P(valid);
        write(fd, blk.data, blk.size);
        V(empty);
        std::cout << "writebuf: " << idx << std::endl;
        // 判断是否结束
        if (blk.end) {
            break;
        }
        idx = (idx + 1) % BUFCNT;
    }
    std::cout << "writebuf end" << std::endl;
    return 0;
}
