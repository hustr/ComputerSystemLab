#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <iomanip>
#include <chrono>
#include <ctime>

#define NANO 1000000000

static const std::map<const int, const std::string> type_to_string{
        {DT_BLK,     "block device"},
        {DT_CHR,     "character device"},
        {DT_DIR,     "directory"},
        {DT_FIFO,    "named pipe"},
        {DT_LNK,     "symbolic link"},
        {DT_REG,     "regular file"},
        {DT_SOCK,    "UNIX domain socket"},
        {DT_UNKNOWN, "UNKNOWN"},
};

void print_info(const std::string &path) {
    // 输出路径对应的文件或目录的详细信息
    struct stat stat_buf{};
    // 打开目录项
    if(lstat(path.c_str(), &stat_buf) < 0) {
        std::cerr << "errno: " << errno << std::endl;
        return;
    }
    // 输出文件类型
    mode_t &mode = stat_buf.st_mode;
    if (S_ISREG(mode)) {
        std::cout << '-';
    } else if (S_ISDIR(mode)) {
        std::cout << 'd';
    } else if (S_ISCHR(mode)) {
        std::cout << 'c';
    } else if (S_ISBLK(mode)) {
        std::cout << 'b';
    } else if (S_ISLNK(mode)) {
        std::cout << 'l';
    }  else if (S_ISSOCK(mode)) {
        std::cout << 's';
    }
    // 用户读写执行权限
    std::cout << ((S_IRUSR & mode ) ? 'r' : '-');
    std::cout << ((S_IWUSR & mode ) ? 'w' : '-');    
    std::cout << ((S_IXUSR & mode ) ? 'x' : '-');
    // 组用户权限    
    std::cout << ((S_IRGRP & mode ) ? 'r' : '-');
    std::cout << ((S_IWGRP & mode ) ? 'w' : '-');    
    std::cout << ((S_IXGRP & mode ) ? 'x' : '-');
    // 其他用户权限
    std::cout << ((S_IROTH & mode ) ? 'r' : '-');
    std::cout << ((S_IWOTH & mode ) ? 'w' : '-');    
    std::cout << ((S_IXOTH & mode ) ? 'x' : '-');
    // 输出文件硬链接数
    std::cout << ' ' << stat_buf.st_nlink << ' ';
    // 输出用户名和组名
    passwd *t = getpwuid(stat_buf.st_uid);
    std::cout << t->pw_name << ' ';
    group *grp = getgrgid(stat_buf.st_gid);
    std::cout << grp->gr_name << ' ';
    // 输出文件大小
    // 暂时不转换文件的大小到MB,GB了
    std::cout << stat_buf.st_size << ' ';
    // 输出文件最后修改时间
    auto str = std::put_time(std::localtime(&stat_buf.st_mtim.tv_sec), "%Y-%m-%d %H:%M:%S");
    std::cout << str << ' ';
    // 输出文件名
    size_t idx = path.rfind('/');
    if (idx != std::string::npos && idx != path.length() - 1) {
        std::cout << path.substr(idx + 1);
    } else {
        std::cout << path;
    }
}

void printdir(const std::string &dirpath, int depth = 0) {
    const static std::string cur = ".", pre = "..";

    DIR *dir = opendir(dirpath.c_str());
    auto del_dir = [&](DIR *dir) {
        // std::cout << "close dir " << dirpath << ", errno: " << closedir(dir) << std::endl;
        // std::cout << std::endl;
        closedir(dir);
    };
    if (dir == nullptr) {
        std::cerr << "error open dir, errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    // 确保dir最后一定会被关闭
    std::unique_ptr<DIR, decltype(del_dir)> dir_guard(dir, del_dir);

    dirent *ent;
    std::string filename = dirpath;
    if (filename.back() != '/') {
        filename.push_back('/');
    }
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_name == cur || ent->d_name == pre) {
            // 跳过当前目录和上级目录
            continue;
        }
        // 空白总是要先输出的
        for (int i = 0; i < depth; ++i) {
            std::cout << ' ';
        }
        print_info(filename + ent->d_name);
        std::cout << std::endl;
        if (ent->d_type == DT_DIR) {
            printdir(filename + ent->d_name, depth + 4);
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            printdir(argv[i]);
        }
    } else {
        printdir(".");
    }


    return 0;
}
