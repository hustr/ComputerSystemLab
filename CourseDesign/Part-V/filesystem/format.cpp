#include <iostream>
#include "rammiahfs/RFS.hpp"

int main(int argc,char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <file name> <block count>" << "\n";
        return -1;
    }

    int blk_cnt = std::stod(argv[2]);
    if(RFS::create_disk_file(argv[1], blk_cnt)) {
        std::cout << "create file " << argv[1] << "success\n";
    } else {
        std::cout << "create file " << argv[1] << "failed\n";
    }



    return 0;
}

