#include "rammiahfs/RFS.hpp"


int main(int argc, char *argv[]) {
//    if (argc < 3) {
//        std::cout << "usage: ./format <name> <block_cnt>\n";
//        return -1;
//    }
//    RFS::create_disk_file("data.dat", 40960);
    if (argc < 2) {
        std::cout << "usage: ./filesystem <binary file>" << std::endl;
        return 0;
    }

    RFS rfs(argv[1]);

    std::string comm;

    while (true) {
        std::cout << ">>>";
//        std::getline(std::cin, comm);
        std::cin >> comm;
        if (comm == "exit") {
            std::cout << "goodbye!\n";
            break;
        } else if (comm == "ls") {
            // list all files in
            auto files = rfs.list_files();
            for (auto &file : files) {
                std::cout << file.second << "\t" << file.first << "\n";
            }
        } else if (comm == "rm") {
            std::string file;
            std::cin >> file;
//            while (std::cin >> file) {
            std::cout << "rm " << file << " " << (rfs.rm_file(file.c_str()) ? "success" : "failed") << std::endl;
//            }
        } else if (comm == "open") {
            // get a file descriptor
            // file name and operation type
            std::string file, type;
            std::cin >> file >> type;
            if (type == "r") {
                std::cout << rfs.open(file.c_str(), omode_t::READ) << std::endl;
            } else if (type == "w") {
                std::cout << rfs.open(file.c_str(), omode_t::WRITE) << std::endl;
            } else if (type == "a") {
                std::cout << rfs.open(file.c_str(), omode_t::APPEND) << std::endl;
            } else {
                std::cout << "open failed, please check if file exists or is opened" << std::endl;
            }
        } else if (comm == "close") {
            int32_t fd;
            std::cin >> fd;
            std::cout << "close fd " << fd << (rfs.close(fd) ? " success" : " failed") << std::endl;
        } else if (comm == "read") {
            // read fd and len
            int32_t fd, len;
            std::cin >> fd >> len;
            std::unique_ptr<char[]> buf = std::make_unique<char[]>(len + 1);
            int32_t read_len = rfs.read(fd, buf.get(), len);
            if (read_len < 0) {
                std::cout << "read failed, please check fd\n";
            } else {
                std::cout << "read len = " << read_len << std::endl;
                buf[read_len] = '\0';
                std::cout << buf.get() << "\n";
            }
        } else if (comm == "write") {
            int32_t fd, len;
            std::cin >> fd >> len;
            std::string content;
            std::cin >> content;
            int32_t write_len = rfs.write(fd, content.c_str(), len);
            if (write_len >= 0) {
                std::cout << "write len: " << write_len << std::endl;
            } else {
                std::cout << "write failed, please check fd" << std::endl;
            }
        } else if (comm == "cp") {
            std::string src, dst;
            std::cin >> src >> dst;
            int32_t src_fd = rfs.open(src.c_str(), omode_t::READ);
            if (src_fd < 0) {
                std::cout << "please enter valid src file" << std::endl;
                continue;
            }
            int32_t dst_fd = rfs.open(dst.c_str(), omode_t::WRITE);
            if (dst_fd < 0) {
                std::cout << "please enter valid dst file" << std::endl;
                rfs.close(src_fd);
                continue;
            }
            char buf[100];
            int len;
            while ((len = rfs.read(src_fd, buf, sizeof(buf))) > 0) {
                rfs.write(dst_fd, buf, len);
            }

            rfs.close(src_fd);
            rfs.close(dst_fd);
        }
    }


    return 0;
}

