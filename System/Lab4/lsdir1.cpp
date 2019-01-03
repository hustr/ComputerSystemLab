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
#include <stack>

typedef std::string path_t;
typedef int depth_t;

void print_space(depth_t depth) {
    if (depth > 0) {
        std::cout << std::setw(depth) << std::right << ' ';
    }
}

void print_info(const std::string &path) {
    // 输出路径对应的文件或目录的详细信息
    struct stat stat_buf{};
    // 打开目录项
    if (lstat(path.c_str(), &stat_buf) < 0) {
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
    } else if (S_ISSOCK(mode)) {
        std::cout << 's';
    }
    // 用户读写执行权限
    std::cout << ((S_IRUSR & mode) ? 'r' : '-');
    std::cout << ((S_IWUSR & mode) ? 'w' : '-');
    std::cout << ((S_IXUSR & mode) ? 'x' : '-');
    // 组用户权限
    std::cout << ((S_IRGRP & mode) ? 'r' : '-');
    std::cout << ((S_IWGRP & mode) ? 'w' : '-');
    std::cout << ((S_IXGRP & mode) ? 'x' : '-');
    // 其他用户权限
    std::cout << ((S_IROTH & mode) ? 'r' : '-');
    std::cout << ((S_IWOTH & mode) ? 'w' : '-');
    std::cout << ((S_IXOTH & mode) ? 'x' : '-');
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

void lsdir(const std::string &dirpath) {
    const static std::string cur = ".", pre = "..";
    std::stack<std::pair<path_t, depth_t>> dirs;
    dirs.push(std::make_pair(dirpath, 0));
    auto del_dir = [&](DIR *dir) {
        closedir(dir);
    };
    while (!dirs.empty()) {
        auto curpair = std::move(dirs.top());
        dirs.pop();
        std::string &curdirpath = curpair.first;
        if (curdirpath != ".") {
            print_space(curpair.second - 4);
            print_info(curdirpath);
            std::cout << std::endl;
        }

        if (curdirpath.back() != '/') {
            curdirpath.push_back('/');
        }

        DIR *dir = opendir(curdirpath.c_str());
        if (dir == nullptr) {
            std::cerr << "error open dir, errno: " << errno << std::endl;
            continue;
        }
        // 确保dir最后一定会被关闭
        std::unique_ptr<DIR, decltype(del_dir)> dir_guard(dir, del_dir);

        dirent *ent;
        // 记录一下文件，应该文件先显示，目录再显示
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_name == cur || ent->d_name == pre) {
                // 跳过当前目录和上级目录
                continue;
            }
            if (ent->d_type == DT_DIR) {
                dirs.push(std::make_pair(curdirpath + ent->d_name, curpair.second + 4));
            } else {
                print_space(curpair.second);
                print_info(curdirpath + ent->d_name);
                std::cout << std::endl;
            }
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            lsdir(argv[i]);
        }
    } else {
        lsdir(".");
    }

    return 0;
}
