#include "config.hpp"

int main(int argc, char *argv[]) {
    // put直接打开原文件
    std::string filename(argv[0]);
    std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
    if (!file.is_open()) {
        std::cerr << "file " << filename << " open failed" << std::endl;
        exit(EXIT_FAILURE);
     }
    // 获取虚拟内存块
    int shm = shmget(SHM_ID, BUFCNT * sizeof(Block), 0666);
    Block *blocks = static_cast<Block*>(shmat(shm, nullptr, 0));
    // 获取信号灯
    int sem = semget(SEM_ID, SEM_CNT, 0666);
    std::cout << "put sem: " << sem << std::endl;
    // 获取文件大小
    file.seekg(0, std::ios_base::end);
    const unsigned long size = file.tellg();
    file.seekg(0, std::ios_base::beg);
    // 写入文件
    int block_idx = 0;
    char buf[BUFSIZE];
    while (true) {
        P(sem, EMPTY);
        auto &blk = blocks[block_idx];
        // 写入文件
        // 判断是否够读一个块
        // std::cout << "put: " << size << " " << file.tellg() << std::endl;
        if (size - file.tellg() > BUFSIZE) {
            file.read(blk.data, BUFSIZE);
            blk.end = false;
            blk.size = BUFSIZE;
            std::cout << "put: " << block_idx << std::endl;
            V(sem, VALID);
        } else {
            int put_bytes = size - file.tellg();
            file.read(blk.data, put_bytes);
            blk.end = true;
            blk.size = put_bytes;
            std::cout << "put: " <<block_idx << std::endl;
            // 写入结束
            V(sem, VALID);
            break;
        }
        block_idx = (block_idx + 1) % BUFCNT;
    }
    std::cout << "put end" << std::endl;
    shmdt(blocks);
    // 结束写入程序
    return 0;
}
