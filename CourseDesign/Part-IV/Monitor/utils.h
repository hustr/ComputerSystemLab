#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <map>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iostream>
#include <QDebug>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mutex>
#include <numeric>
#include <memory>
#include <dirent.h>
#include <sys/user.h>
#include <pwd.h>



#define TM_FORMAT "%Y-%m-%d %H:%M:%S"
#define MEM_TOTAL "MemTotal"
#define MEM_AVAIL "MemAvail"
#define SWAP_TOTAL "SwapTotal"
#define SWAP_FREE "SwapFree"
#define STAT_FMT "%d (%[^)]) %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu \
%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %d"

#define SECOND 1
#define MINUTE (60 * SECOND)
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)

// store cpu_usage into a variable
extern std::atomic<double> cpu_usage;
extern std::atomic<double> mem_usage;
extern std::atomic<double> swap_usage;


// calculate cpu usage
void update_cpu_usage();

void update_mem_usage();

// get status bar message
char *get_status_msg();

// get system start time
const tm *get_start_time();

// get system uptime
clock_t get_uptime();

const char *get_os_type();

const char *get_host();

const char *get_cpu_model();

// get cpu average frequence in MHz
double get_cpu_freq();

// create a process struct to save process information
struct process_t {
    // content about /proc/$pid/stat
    int pid;
    char comm[100];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long int cutime;
    long int cstime;
    long int priority;
    long int nice;
    long int num_threads;
    long int itrealvalue;
    unsigned long long starttime;
    unsigned long vsize; // bytes
    long int rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned rt_priority;
    unsigned policy;
    unsigned long long int delayacct_blkio_ticks;
    unsigned long int guest_time;
    long int cguest_time;
    unsigned long start_data;
    unsigned long end_data;
    unsigned long start_brk;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long env_start;
    unsigned long env_end;
    int exit_code;
    // content about /proc/$pid/statm
    long int size;
    long int resident;
    long int share;
    long int trs;
    long int lrs;
    long int drs;
    long int dt;
};

process_t get_process_info(pid_t pid, bool *ok=nullptr);

// get all processes' pid
std::vector<pid_t> get_pids();

std::unique_ptr<char[]> calculate_memory(const process_t &p);

std::unique_ptr<char[]> get_user(const pid_t &id);

const char *status_char_to_str(char status);

#endif // UTILS_H
