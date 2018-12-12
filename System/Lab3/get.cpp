#include "config.hpp"

int main(int argc, char *argv[]) {
    assert(argc == 1);
    // get创建一个copy文件
    std::string filename(argv[0]);
    filename.append(".cpy");
    // 获取信号灯，如何判断结束是个问题
    // 二进制写入
    std::ofstream file(filename, std::ios_base::binary | std::ios_base::out);
    // 获取虚拟内存
    int shm = shmget(SHM_ID, BUFCNT * sizeof(Block), 0666);
    Block *blocks = static_cast<Block*>(shmat(shm, nullptr, 0));
    // 获取信号灯
    int block_idx = 0;
    int sem = semget(SEM_ID, SEM_CNT, 0666);
    std::cout << "get sem: " << sem << std::endl;
    while (true) {
        // 写入文件
        std::cout << "get: " << block_idx << std::endl;
        auto &blk = blocks[block_idx];
        P(sem, VALID);
        file.write(blk.data, blk.size);
        V(sem, EMPTY);
        // 判断是否结束
        if (blk.end) {
            break;
        }
        block_idx = (block_idx + 1) % BUFCNT;
    }
    std::cout << "get end" << std::endl;
    shmdt(blocks);
    return 0;
}
