//
// Created by yaning on 19-3-11.
//

#include "RFS.hpp"

bool RFS::read_blk(int32_t id, RFS::block_t &blk) {
//    printf("read begin: %lu\n", id * sizeof(blk) + skip);
    fseek(file_ptr, (id + sblk_cnt) * sizeof(blk), SEEK_SET);
    fread(&blk, sizeof(block_t), 1, file_ptr);
    return true;
}

bool RFS::write_blk(int32_t id, const RFS::block_t &blk) {
//    printf("write begin: %lu\n", id * sizeof(blk) + skip);
    fseek(file_ptr, (id + sblk_cnt) * sizeof(blk), SEEK_SET);
    fwrite(&blk, sizeof(block_t), 1, file_ptr);
    fflush(file_ptr);
    return true;
}

bool RFS::add_to_unused(const int32_t &id) {
    blks[free_blk.end] = id;
    blks[id] = -1;
    free_blk.end = id;
    // 文件写满了之后来了个可用磁盘块，查看start是否需要自己更新
    if (free_blk.start == -1) {
        free_blk.start = free_blk.end;
    }
    return true;
}

bool RFS::get_unused(int32_t &id) {
    if (free_blk.start == -1) {
        return false;
    }
    id = free_blk.start;
    // 轮换到下一个块
    free_blk.start = blks[free_blk.start];
    return true;
}

int32_t RFS::read(int32_t file_id, char *const buf, int len) {
    // 这个先判断文件是否打开，并且格式是否对
    auto it = opened_map.find(file_id);
    if (it != opened_map.end() && it->second.mode == omode_t::READ) {
        // 可以读
        auto &f = opened_map.at(file_id);
        int32_t &off = f.offset;
        // calculate blk_idx index
        int32_t blk_idx = off / BLK_SIZE;
        int32_t need_read = len;
        if (off + len > files[file_id].size) {
            need_read = files[file_id].size - off;
        }
        // just read need_read
        // may be need several blk_idx
        std::vector<int32_t> &all_blk = f.all_blk;
//        int32_t start = files[file_id].start_blk;
//        while (start != -1) {
//            all_blk.push_back(start);
//            start = blks[start];
//        }
        block_t blk{};
        int32_t readed = 0;
        while (readed != need_read) {
            read_blk(all_blk[blk_idx], blk);
            // how many and start idx
            if ((off + readed) % BLK_SIZE == 0) {
                // blk start
                if (need_read - readed >= BLK_SIZE) {
                    // just read all
                    memcpy(buf + readed, blk.data, BLK_SIZE);
                    readed += BLK_SIZE;
                } else {
                    // not read all
                    memcpy(buf + readed, blk.data, static_cast<size_t>(need_read - readed));
                    // read finished
                    readed = need_read;
                }
            } else {
                // not a full block, just the first
                if (off + need_read >= (blk_idx + 1) * BLK_SIZE) {
                    // read all
                    memcpy(buf + readed, blk.data + off % BLK_SIZE, static_cast<size_t>(BLK_SIZE - off % BLK_SIZE));
                    readed += BLK_SIZE - off % BLK_SIZE;
                } else {
                    // read needed
                    memcpy(buf + readed, blk.data + off % BLK_SIZE, static_cast<size_t>(need_read));
                }
            }
            ++blk_idx;
        }
        off += readed;
        return readed;
    }
    // 不允许读操作
    return -1;
}

int32_t RFS::open(const char *const filename, omode_t m) {
    int32_t id = -1;
    if (m == omode_t::WRITE) {
        // 删除旧文件，建立新文件
        rm_file(filename);
        if (creat_file(filename)) {
            // 直接使用文件在数组中的下标怎么样
            for (int i = 0; i < FILE_MAX_CNT; ++i) {
                if (files[i].used && strcmp(files[i].name, filename) == 0) {
                    if (opened_map.count(i) > 0) {
                        std::cout << "please close file " << filename << "first\n";
                        return id;
                    }
                    id = i;
                    // 在map中注册一下
                    // rm_file中已经将start_blk置为-1
                    opened_map[id] = opened_file_t{.offset = 0, .mode = omode_t::WRITE};
                    return id;
                }
            }
        }
    } else if (m == omode_t::READ) {
        // 读取操作
        for (int i = 0; i < FILE_MAX_CNT; ++i) {
            if (files[i].used && strcmp(files[i].name, filename) == 0) {
                if (opened_map.count(i) > 0) {
                    std::cout << "please close file " << filename << "first\n";
                    return id;
                }
                id = i;
                int32_t start = files[id].start_blk;
                opened_map[i] = opened_file_t{.offset = 0, .mode = omode_t::READ};
                auto &all_blk = opened_map[i].all_blk;
                while (start != -1) {
                    all_blk.push_back(start);
                    start = blks[start];
                }
                return id;
            }
        }
    } else if (m == omode_t::APPEND) {
        // 追加写
        for (int i = 0; i < FILE_MAX_CNT; ++i) {
            if (files[i].used && strcmp(files[i].name, filename) == 0) {
                if (opened_map.count(id) > 0) {
                    std::cout << "please close file " << filename << "first\n";
                    return id;
                }
                id = i;
                opened_map[id] = opened_file_t{.offset = files[i].size, .mode = omode_t::APPEND};
                int32_t start = files[id].start_blk;
                auto &all_blk = opened_map[i].all_blk;
                while (start != -1) {
                    all_blk.push_back(start);
                    start = blks[start];
                }
                return id;
            }
        }
    }
    return id;
}

bool RFS::close(int32_t file_id) {
    if (opened_map.count(file_id)) {
        opened_map.erase(file_id);
        return true;
    }

    return false;
}

// 写操作
int32_t RFS::write(int32_t file_id, const char *buf, int32_t len) {
    auto it = opened_map.find(file_id);
    if (it != opened_map.end() && (it->second.mode == omode_t::WRITE || it->second.mode == omode_t::APPEND)) {
        // 可以读
        auto &f = it->second;
        int32_t &off = f.offset;
        // calculate blk_idx index

        int32_t need_write = len;
        // just read need_read
        // may be need several blk_idx
        std::vector<int32_t> &all_blk = f.all_blk;
        int32_t start = files[file_id].start_blk;
//        while (start != -1) {
//            all_blk.push_back(start);
//            start = blks[start];
//        }
        int32_t blk_idx = off / BLK_SIZE;
        // 64000 bytes, incorrect blk_idx
        if (off % BLK_SIZE == 0) {
            --blk_idx;
        }
//        int32_t blk_idx = all_blk.size() - 1;
        // maybe all_blk is empty
        int32_t wrote = 0;
        block_t blk{};
        while (wrote != need_write) {
            if ((off + wrote) % BLK_SIZE == 0) {
                // need a new blk
                int32_t id;
                if (!get_unused(id)) {
                    std::cout << "get unused blk failed\n";
                    return 0;
                }
                if (all_blk.empty()) {
                    all_blk.push_back(id);
                    files[file_id].start_blk = id;
                    blk_idx = 0;
                    // next blk is -1
                } else {
                    all_blk.push_back(id);
                    blks[all_blk[blk_idx]] = id;
//                    std::cout << "before: " << blk_idx << " ";
                    blk_idx++;
//                    std::cout << "after: " << blk_idx << "\n";
                }
                blks[id] = -1;
//                std::cout << all_blk.size() << " " << blk_idx << "\n";
                if (off + need_write >= (blk_idx + 1) * BLK_SIZE) {
                    memcpy(blk.data, buf + wrote, BLK_SIZE);
                    write_blk(all_blk[blk_idx], blk);
                    wrote += BLK_SIZE;
                } else {
                    // write left, need write - wrote
                    memcpy(blk.data, buf + wrote, static_cast<size_t>(need_write - wrote));
                    write_blk(all_blk[blk_idx], blk);
                    wrote = need_write;
                }
            } else {
                // must not be first blk
                read_blk(all_blk[blk_idx], blk);
                // out of this block
                if (off + need_write >= (blk_idx + 1) * BLK_SIZE) {
                    // read back and write
                    memcpy(blk.data + (off + wrote) % BLK_SIZE, buf + wrote,
                           static_cast<size_t>(BLK_SIZE - (off + wrote) % BLK_SIZE));
                    write_blk(all_blk[blk_idx], blk);
                    wrote += BLK_SIZE - (off + wrote) % BLK_SIZE;
                } else {
                    memcpy(blk.data + (off + wrote) % BLK_SIZE, buf + wrote, static_cast<size_t>(need_write - wrote));
                    write_blk(all_blk[blk_idx], blk);
                    wrote = need_write;
                }
            }
//            std::cout << "blk_idx = " << blk_idx << "\n";
        }

        off += wrote;
//        std::cout << "off = " << off << " ";
//        std::cout << off / BLK_SIZE << " " << blk_idx << "\n";
        files[file_id].size = off;
        return wrote;
    }
    // 不允许读操作
    return -1;
}

std::vector<std::pair<std::string, int32_t>> RFS::list_files() {
    std::vector<std::pair<std::string, int32_t>> res;
    for (auto &file : files) {
        if (file.used) {
            res.emplace_back(file.name, file.size);
        }
    }

    return res;
}

bool RFS::rm_file(const char *name) {
    for (auto &file : files) {
        if (file.used && strcmp(name, file.name) == 0) {
            // 获取文件的起始块
            int start = file.start_blk;
            // 将块回收
            while (start != -1) {
                int next = blks[start];
                add_to_unused(start);
                start = next;
            }
            file.size = 0;
            file.start_blk = -1;
            file.used = false;
            bzero(file.name, sizeof(file.name));
            return true;
        }
    }
    return false;
}

// 不允许重名文件出现
bool RFS::creat_file(const char *filename) {
    // 我才没有时间判断你是否存在了
    rm_file(filename);
    // 查找一个可用的文件位置
    // 如果没地方写的话就没办法了
    for (auto &file : files) {
        if (!file.used) {
            // 创建文件
            file.used = true;
            file.start_blk = -1;
            file.size = 0;
            strcpy(file.name, filename);
            return true;
        }
    }

    return false;
}

RFS::RFS(const char *filename) {
    file_ptr = fopen(filename, "r+");
    int32_t id = 0;
    sblk_cnt = 0;
    block_t blk{};
    read_blk(id, blk);
    int32_t cnt = 0;
    memcpy(&cnt, blk.data, sizeof(sblk_cnt));
    std::unique_ptr<char[]> read_buf = std::make_unique<char[]>(cnt * BLK_SIZE);
    for (int i = 0; i < cnt; ++i) {
        read_blk(i, blk);
        memcpy(read_buf.get() + i * BLK_SIZE, blk.data, sizeof(blk.data));
    }
//    sblk_cnt = cnt;

    // 读入元数据
//    skip = 0;
    // 4字节数据块数目
    int32_t bytes = 0;
    memcpy(&sblk_cnt, read_buf.get() + bytes, sizeof(sblk_cnt));
    bytes += sizeof(sblk_cnt);
    memcpy(&blk_cnt, read_buf.get() + bytes, sizeof(blk_cnt));
    bytes += sizeof(blk_cnt);
    // 读入free_blk起始和结束
    memcpy(&free_blk.start, read_buf.get() + bytes, sizeof(free_blk.start));
    bytes += sizeof(free_blk.start);
    memcpy(&free_blk.end, read_buf.get() + bytes, sizeof(free_blk.end));
    bytes += sizeof(free_blk.end);
    // 读入blks数据
    blks = std::make_unique<int32_t[]>(blk_cnt);
    memcpy(blks.get(), read_buf.get() + bytes, sizeof(blks[0]) * blk_cnt);
    bytes += blk_cnt * sizeof(blks[0]);
    // 读入文件数据
    memcpy(files, read_buf.get() + bytes, sizeof(file_t) * FILE_MAX_CNT);
}

RFS::~RFS() {
    //  把信息写回文件
//    fseek(file_ptr, 0, SEEK_SET);
//    fwrite(&blk_cnt, sizeof(blk_cnt), 1, file_ptr);
//    fwrite(&free_blk.start, sizeof(free_blk.start), 1, file_ptr);
//    fwrite(&free_blk.end, sizeof(free_blk.end), 1, file_ptr);
//    fwrite(blks.get(), sizeof(blks[0]), blk_cnt, file_ptr);
//    fwrite(files, sizeof(file_t), FILE_MAX_CNT, file_ptr);
    // 每个磁盘块中的内容不需要管
    std::unique_ptr<char[]> write_buf = std::make_unique<char[]>(sblk_cnt * BLK_SIZE);
    int32_t bytes = 0;
    memcpy(write_buf.get() + bytes, &sblk_cnt, sizeof(sblk_cnt));
    bytes += sizeof(sblk_cnt);
    memcpy(write_buf.get() + bytes, &blk_cnt, sizeof(blk_cnt));
    bytes += sizeof(blk_cnt);
    memcpy(write_buf.get() + bytes, &free_blk.start, sizeof(free_blk.start));
    bytes += sizeof(free_blk.start);
    memcpy(write_buf.get() + bytes, &free_blk.end, sizeof(free_blk.end));
    bytes += sizeof(free_blk.end);
    memcpy(write_buf.get() + bytes, blks.get(), sizeof(blks[0]) * blk_cnt);
    bytes += sizeof(blks[0]) * blk_cnt;
    memcpy(write_buf.get() + bytes, files, sizeof(file_t) * FILE_MAX_CNT);
    // ok end
    block_t blk{};
    int cnt = sblk_cnt;
    sblk_cnt = 0;
//    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < cnt; ++i) {
        memcpy(&blk, write_buf.get() + sizeof(block_t) * i, sizeof(block_t));
//        fwrite(&blk, sizeof(block_t), 1, file_ptr);
        write_blk(i, blk);
    }

    fclose(file_ptr);
}

bool RFS::create_disk_file(const char *filename, const int32_t blk_cnt) {
    FILE *fp = fopen(filename, "w");
    int bytes = 0;

    std::unique_ptr<int32_t[]> blks = std::make_unique<int32_t[]>(blk_cnt);
    for (int i = 0; i < blk_cnt - 1; ++i) {
        blks[i] = i + 1;
    }
    blks[blk_cnt - 1] = -1;
    free_blk_t free_blk{.start = 0, .end = blk_cnt - 1};
    file_t files[FILE_MAX_CNT];
    for (auto &f : files) {
        f.start_blk = -1;
        f.size = 0;
        f.used = false;
    }

    fseek(fp, 0, SEEK_SET);
    bytes += sizeof(int32_t); // super block cnt
    bytes += sizeof(blk_cnt);
    bytes += sizeof(free_blk.start);
    bytes += sizeof(free_blk.end);
    bytes += sizeof(blks[0]) * blk_cnt;
    bytes += sizeof(file_t) * FILE_MAX_CNT;
    int32_t need_blk = bytes / BLK_SIZE + 1; // 123 need 1
    if (bytes % BLK_SIZE == 0) {
        need_blk--;
    }
    std::unique_ptr<char[]> write_buf = std::make_unique<char[]>(BLK_SIZE * need_blk);
    bytes = 0;
    memcpy(write_buf.get() + bytes, &need_blk, sizeof(need_blk));
    bytes += sizeof(need_blk);
    memcpy(write_buf.get() + bytes, &blk_cnt, sizeof(blk_cnt));
    bytes += sizeof(blk_cnt);
    memcpy(write_buf.get() + bytes, &free_blk.start, sizeof(free_blk.start));
    bytes += sizeof(free_blk.start);
    memcpy(write_buf.get() + bytes, &free_blk.end, sizeof(free_blk.end));
    bytes += sizeof(free_blk.end);
    memcpy(write_buf.get() + bytes, blks.get(), sizeof(blks[0]) * blk_cnt);
    bytes += sizeof(blks[0]) * blk_cnt;
    memcpy(write_buf.get() + bytes, files, sizeof(file_t) * FILE_MAX_CNT);
    bytes += sizeof(file_t) * FILE_MAX_CNT;
    // ok end
    block_t blk{};
    for (int i = 0; i < need_blk; ++i) {
        memcpy(&blk, write_buf.get() + sizeof(block_t) * i, sizeof(block_t));
        fwrite(&blk, sizeof(block_t), 1, fp);
    }
    memset(&blk, 0, sizeof(block_t));
    for (int i = 0; i < blk_cnt; ++i) {
        fwrite(&blk, sizeof(block_t), 1, fp);
    }

//    bytes += fwrite(&blk_cnt, sizeof(blk_cnt), 1, fp) * sizeof(blk_cnt);
//    bytes += fwrite(&free_blk.start, sizeof(free_blk.start), 1, fp) * sizeof(free_blk.start);
//    bytes += fwrite(&free_blk.end, sizeof(free_blk.end), 1, fp) * sizeof(free_blk.end);
//    bytes += fwrite(blks.get(), sizeof(blks[0]), blk_cnt, fp) * sizeof(blks[0]);
//    bytes += fwrite(files, sizeof(file_t), FILE_MAX_CNT, fp) * sizeof(file_t);
//    // write blk
//    block_t blk{};
//    for (int i = 0; i < blk_cnt; ++i) {
//        bytes += fwrite(&blk, sizeof(block_t), 1, fp) * sizeof(block_t);
//    }

    fclose(fp);

    return true;
}
