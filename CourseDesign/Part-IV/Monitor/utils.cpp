#include "utils.h"


// define several global data used by graph
std::atomic<double> cpu_usage(0.0);
std::atomic<double> mem_usage(0.0);
std::atomic<double> swap_usage(0.0);

inline void set_bool(bool *ok, const bool &val) {
    if (ok != nullptr) {
        *ok = val;
    }
}


unsigned long long get_total_cpu_time(unsigned long long &work) {
    char buffer[1024];
    int fd = open("/proc/stat", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open file /proc/stat failed: %s\n", strerror(errno));
        return 0;
    }
    read(fd, buffer, sizeof(buffer));
    close(fd);
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
//    // added between Linux 2.5.41 and 2.6.33, see man proc(5)
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    sscanf(buffer,
           "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

    work = user + nice + system;

    // sum everything up (except guest and guestnice since they are already included
    // in user and nice, see http://unix.stackexchange.com/q/178045/20626)
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

void update_cpu_usage() {
    // static variables save the previous data to calculate cpu usage
    static std::atomic<unsigned long long> pre_work(0), pre_total(0);
    unsigned long long work, total;
    // get total
    total = get_total_cpu_time(work);
//    qDebug() << "total " << total << " work " << work << "\n";
    cpu_usage = (work - pre_work) * 100.0 / (total - pre_total);
    pre_total = total;
    pre_work = work;
}

/*
 * update the mem usage by read /proc/meminfo
*/
void update_mem_usage() {
    static std::once_flag once{};
    int fd = open("/proc/meminfo", O_RDONLY);
    if (fd < 0) {
        qDebug() << "open /proc/meminfo failed: " << strerror(errno) << "\n";
        mem_usage = 0;
        swap_usage = 0;
        return;
    }
    static char buf[4096];
    auto read_len = read(fd, buf, sizeof(buf));
    close(fd);
    std::string str(buf, 0, static_cast<size_t>(read_len));
//    qDebug() << read_len;
//    qDebug() << str.c_str() << "\n";
    unsigned long  pos;
    // next we will not modify str
    const char *s_ptr = str.c_str();
    static unsigned long long mem_total{}, swap_total{};
    unsigned long long mem_avail{}, swap_free{};
    // memory total and swap total will only calculate once
    std::call_once(once, [&](){
        if ((pos = str.find(MEM_TOTAL)) != std::string::npos) {
            sscanf(s_ptr + pos, "MemTotal: %llu kB", &mem_total);
            qDebug() << "mem total " << mem_total;
        }
        if ((pos = str.find(SWAP_TOTAL)) != std::string::npos) {
            sscanf(s_ptr + pos, "SwapTotal: %llu kB", &swap_total);
            qDebug() << "swap total " << swap_total;
        }
//        qDebug() << "mem total " << mem_total << " swap total "
    });

    if ((pos = str.find(MEM_AVAIL)) != std::string::npos) {
        sscanf(s_ptr + pos, "MemAvailable: %llu kB", &mem_avail);
    }

    if ((pos = str.find(SWAP_FREE)) != std::string::npos) {
        sscanf(s_ptr + pos, "SwapFree: %llu kB", &swap_free);
    }
//    sprintf(buf, "mem_total %llu MB, mem_available %llu MB, swap_total %llu MB, swap_free %llu MB\n"
//                 "mem_usage %.2f%%, swap_usage %.2f%%",
//            mem_total / 1024, mem_avail / 1024, swap_total / 1024, swap_free / 1024,
//            100 - mem_avail * 100.0 / mem_total, (swap_total == 0 ? 0.0 : 100 - swap_free * 100.0 / swap_total));
//    qDebug() << buf;
    mem_usage = (mem_total == 0 ? 0 :  100 - mem_avail * 100.0 / mem_total);
    swap_usage = (swap_total == 0 ? 0.0 : 100 - swap_free * 100.0 / swap_total);
}

// status message
char *get_status_msg() {
    static char msg[1024];
    time_t t = time(nullptr);
    tm *tm_ptr = localtime(&t);
    static char time_str[100];
    strftime(time_str, sizeof(time_str) - 1, TM_FORMAT, tm_ptr);

    sprintf(msg, "%s CPU %.2f%%, Mem %.2f%%", time_str, cpu_usage.load(), mem_usage.load());
    return msg;
}


const tm *get_start_time() {
    static std::once_flag once;
    static tm t;
    std::call_once(once, [&](){
        int fd = open("/proc/stat", O_RDONLY);
        if (fd < 0) {
            qDebug() << "open /proc/stat failed: " << strerror(errno);
            return;
        }
        // the file is a little big
        char buf[10240]{};
        auto len = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        std::string str(buf, 0, static_cast<unsigned>(len));
        unsigned long pos;
        time_t start{};
        if ((pos = str.find("btime")) != std::string::npos) {
            sscanf(str.c_str() + pos, "btime %ld", &start);
        }
        qDebug() << "start time: " << start;
        t = *localtime(&start);
    });

    return &t;
}

// get uptime
clock_t get_uptime() {
    double up, idle;
    int fd = open("/proc/uptime", O_RDONLY);
    if (fd < 0) {
        qDebug() << "open /proc/uptime failed: " << strerror(errno);
        return 0;
    }
    char buf[100]{};
    read(fd, buf, sizeof(buf) - 1);
    close(fd);
    sscanf(buf, "%lf %lf", &up, &idle);

    return static_cast<clock_t>(up);
}

const char *get_host() {
    static char host[100];
    static std::once_flag once{};
    std::call_once(once, [&](){
        gethostname(host, sizeof(host) - 1);
    });
    return host;
}


const char *get_os_type()
{
    static char os[100];
    static std::once_flag once;
    std::call_once(once, [&](){
         int typefd = open("/proc/sys/kernel/ostype", O_RDONLY);
         if (typefd < 0) {
             qDebug() << "open /proc/sys/kernel/ostype failed: " << strerror(errno);
             return;
         }
         int releasefd = open("/proc/sys/kernel/osrelease", O_RDONLY);
         if (releasefd < 0) {
             qDebug() << "open /proc/sys/kernel/osrelease failed: " << strerror(errno);
             close(typefd);
             return;
         }
         std::string str;
         char buf[50]{};
         long int len = read(typefd, buf, sizeof(buf) - 1);
         str.append(buf, static_cast<size_t>(len));
         // change the '\n' to ' '
         str.back() = ' ';
         len = read(releasefd, buf, sizeof(buf) - 1);
         str.append(buf, static_cast<size_t>(len));
         str.pop_back();

         close(typefd);
         close(releasefd);
         qDebug() << "os: " << str.c_str();
         memcpy(os, str.c_str(), str.size());
    });

    return os;
}

const char *get_cpu_model() {
    static std::once_flag once{};
    static char cpu_model[100]{};
    std::call_once(once, [&](){
        int fd = open("/proc/cpuinfo", O_RDONLY);
        if (fd < 0) {
            qDebug() << "open /proc/cpuinfo failed: " << strerror(errno);
            return;
        }
        size_t buf_size = 20 * 1024;
        // the file is bigger than I think
        std::unique_ptr<char[]> buf = std::make_unique<char[]>(buf_size);
        long int len = read(fd, buf.get(), buf_size);
        close(fd);
        std::string str;
        str.append(buf.get(), static_cast<size_t>(len));
        unsigned long pos;
        if ((pos = str.find("model name")) != std::string::npos) {
            sscanf(str.c_str() + pos, "model name	: %[^\n]", cpu_model);
        }
        qDebug() << "cpu model "<< cpu_model;
    });

    return cpu_model;
}


double get_cpu_freq() {
    //read /proc/cpuinfo to calculate the average frequence of cpu
    int fd = open("/proc/cpuinfo", O_RDONLY);
    if (fd < 0) {
        qDebug() << "open /proc/cpuinfo failed: " << strerror(errno);
        return 0;
    }
    size_t buf_size = 200 * 1024;
    // the file is bigger than I think
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(buf_size);
    long int len = read(fd, buf.get(), buf_size);
    close(fd);
    std::string str;
    str.append(buf.get(), static_cast<size_t>(len));
    unsigned long pos = 0;
    double sum{}, freq;
    int cnt{};
    while ((pos = str.find("cpu MHz", pos)) != std::string::npos) {
        sscanf(str.c_str() + pos, "cpu MHz		: %lf", &freq);
        ++cnt;
        ++pos;
        sum += freq;
    }
    return sum / std::max(1, cnt);
}

process_t get_process_info(pid_t pid, bool *ok) {
    // we have to check if the process is running
    process_t res{};

//    FILE *file = fopen("/proc/1/stat", "r");
    char filename[100]{};
    sprintf(filename, "/proc/%d/stat", pid);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", filename, strerror(errno));
        set_bool(ok, false);
        return res;
    }
    char buf[1024]{};
    read(fd, buf, sizeof(buf));

    sscanf(buf, STAT_FMT, &res.pid, res.comm, &res.state, &res.ppid, &res.pgrp, &res.session, &res.tty_nr, &res.tpgid,
           &res.flags, &res.minflt, &res.cminflt, &res.majflt, &res.cmajflt, &res.utime, &res.stime, &res.cutime,
           &res.cstime, &res.priority, &res.nice, &res.num_threads, &res.itrealvalue, &res.starttime, &res.vsize,
           &res.rss, &res.rsslim, &res.startcode, &res.endcode, &res.startstack, &res.kstkesp, &res.kstkeip,
           &res.signal, &res.blocked, &res.sigignore, &res.sigcatch, &res.wchan, &res.nswap, &res.cnswap,
           &res.exit_signal, &res.processor, &res.rt_priority, &res.policy, &res.delayacct_blkio_ticks, &res.guest_time,
           &res.cguest_time, &res.start_data, &res.end_data, &res.start_brk, &res.arg_start, &res.arg_end,
           &res.env_start, &res.env_end, &res.exit_code);
    // read shared memory
    close(fd);
    sprintf(filename, "/proc/%d/statm", pid);
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", filename, strerror(errno));
        set_bool(ok, false);
        return res;
    }
    read(fd, buf, sizeof(buf));
    sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld", &res.size, &res.resident, &res.share, &res.trs, &res.lrs, &res.drs,
           &res.dt);
    close(fd);
    set_bool(ok, true);

    return res;
}

inline bool is_number(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

// get all processes' pid
std::vector<pid_t> get_pids() {
    std::vector<pid_t> pids;

    DIR *dir = opendir("/proc");
    if (dir == nullptr) {
        fprintf(stderr, "open dir /proc failed: %s\n", strerror(errno));
        return pids;
    }

    dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // check and skip current dir, parent dir
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (entry->d_type == DT_DIR && is_number(entry->d_name)) {
            pids.push_back(std::stoi(entry->d_name));
        }
    }
    closedir(dir);
    // nrvo
    return pids;
}

std::unique_ptr<char[]> calculate_memory(const process_t &p)
{
    // 40 chars should be enough
    std::unique_ptr<char[]> mem = std::make_unique<char[]>(40);
    unsigned long bytes = (p.resident - p.share) * PAGE_SIZE;
    if (bytes >= 1024 * 1024) {
        // transfer to MB
        sprintf(mem.get(), "%.2f MB", bytes / 1024.0 / 1024.0);
    } else {
        // KB
        sprintf(mem.get(), "%.2f KB", bytes / 1024.0);
    }
    return mem;
}

std::unique_ptr<char[]> get_user(const pid_t &id)
{
    // open /proc/$pid/status
    char filename[100]{};
    sprintf(filename, "/proc/%d/status", id);
    std::unique_ptr<char[]> user = std::make_unique<char[]>(100);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open /proc/%d/status failed: %s", id, strerror(errno));
        return user;
    }
    char buf[1024 * 10]{};
    std::string str;
    long int len = read(fd, buf, sizeof(buf));
    close(fd);
    str.append(buf, static_cast<size_t>(len));
    auto pos = str.find("Uid");
    uid_t uid = 0;
    if (pos != std::string::npos) {
        sscanf(str.c_str() + pos, "Uid: %d", &uid);
    }
    passwd *pwd = getpwuid(uid);
    memcpy(user.get(), pwd->pw_name, strlen(pwd->pw_name));
    return user;
}

const char *status_char_to_str(char status)
{
    switch (status) {
    case 'R':
        return "Running";
    case 'S':
        return "Sleeping";
    case 'D':
        return "Waiting";
    case 'Z':
        return "Zombine";
    }

    return "Unknown";
}


































