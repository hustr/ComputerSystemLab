//
// Created by yaning on 19-3-11.
//

#ifndef FILESYSTEM_RFS_HPP
#define FILESYSTEM_RFS_HPP

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

#define BLK_SIZE 512
#define FILE_MAX_CNT 128
#define NAME_LEN 64

enum class omode_t {
    READ, WRITE, APPEND
};

class RFS {
private:
    FILE *file_ptr = nullptr; // 模拟的磁盘文件的文件描述符

    // 磁盘块定义
    struct block_t {
        unsigned char data[BLK_SIZE];
    };

    bool read_blk(int32_t id, block_t &blk);

    bool write_blk(int32_t id, const block_t &blk);

    // 可用磁盘块数目
    uint32_t blk_cnt;
    std::unique_ptr<int32_t[]> blks; // 磁盘块
    struct free_blk_t {
        int32_t start;
        int32_t end;
    } free_blk;

    bool add_to_unused(const int32_t &id);

    bool get_unused(int32_t &id);

    struct file_t {
        bool used;
        char name[NAME_LEN];
        int32_t size;
        int32_t start_blk;
    };
    // 文件最多放128个
    file_t files[FILE_MAX_CNT];
    struct opened_file_t {
        int32_t offset;
        omode_t mode;
        std::vector<int32_t> all_blk;
    };

    // 已打开文件记录
    std::map<int32_t, opened_file_t> opened_map;
    int32_t sblk_cnt;
public:
    explicit RFS(const char *filename);

    ~RFS();

    // 打开文件
    int32_t open(const char *filename, omode_t m);

    int32_t read(int32_t file_id, char *buf, int32_t len);

    int32_t write(int32_t file_id, const char *buf, int32_t len);

    bool close(int32_t file_id);

    // 获取所有文件文件名及文件大小（字节）
    std::vector<std::pair<std::string, int32_t>> list_files();

    bool rm_file(const char *filename);

    bool creat_file(const char *filename);

    static bool create_disk_file(const char *filename, int32_t blk_cnt);
};


#endif //FILESYSTEM_RFS_HPP
