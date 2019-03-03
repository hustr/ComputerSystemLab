#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>

//get rwx mode of file
mode_t get_mode(char *filename) {
    struct stat stat_buf;
    stat(filename, &stat_buf);
    return stat_buf.st_mode;
}


int main(int argc, char *argv[]) {
    // copy file
    if (argc < 3) {
        std::cerr << "copy usage:\n    ./copy <src> <dst>" << std::endl;
        exit(EXIT_FAILURE);
    }
    // try to open file
    int src_fd = open(argv[1], O_RDONLY);
    int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, get_mode(argv[1]));
    if (src_fd <0 || dst_fd <0 ) {
        std::cerr << "open file error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    // create a buff
    char *buf = new char[BUFSIZ];
    int read_len;
    // start copying
    while ((read_len = read(src_fd, buf, BUFSIZ)) > 0) {
        if (write(dst_fd, buf, read_len) < 0) {
            std::cerr << "write failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    delete(buf);

    close(src_fd);
    close(dst_fd);
    std::cout << "copy finished" << std::endl;
    
    return 0;
}
