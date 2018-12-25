# 第三次实验
实验重点是IPC的一种方式：共享内存。

## 数据结构定义
```c++
struct Block {
    // 文件块定义
    bool end{}; // 标志文件结束
    int size{}; // 二进制数据数目
    char data[BUFSIZE];// 数据部分
};
```
## RAII类
```c++
// 内存创建与销毁
template<typename T>
class ShmBlk {
private:
    // 申请到的内存的id
    int id = -1; // 内存的id
    pid_t pid = -1; //进程pid，防止execv失败进行多次释放内存
public:
    ShmBlk() = default;
    // 禁止拷贝和赋值
    ShmBlk(const ShmBlk&) = delete;
    ShmBlk& operator=(const ShmBlk&) = delete;
    ShmBlk(key_t key); // 构造函数保存此key得到的id
    ShmBlk(ShmBlk &&other);// 移动构造获取其他控制块的资源
    ~ShmBlk(); // 释放资源
};

// 内存的attach和detach
template<typename T>
class ShmCtl {
private:
    T *addr = nullptr; // 指向虚拟内存的指针
public:
    ShmCtl() = default;
    // 进制拷贝和赋值
    ShmCtl(const ShmCtl&) = delete;
    ShmCtl& operator=(const ShmCtl&) = delete;
    explicit ShmCtl(int id); // 得到id初始化addr
    ShmCtl(ShmCtl &&other); // 转移资源所有权
    T &get(); // 获取数据
    ~ShmCtl(); // detach内存块
};

// 信号量的创建与销毁
class SemBlk {
private:
    int id = -1; // 信号量id
    pid_t pid = -1; // 进程的id，作用和ShmBlk相同
public:
    // 这个不需要放入vector，默认构造函数也删了
    SemBlk() = delete;
    SemBlk(const SemBlk&) = delete;
    SemBlk &operator=(const SemBlk&) = delete;
    explicit SemBlk(key_t key); // 使用key获取semaphore
    SemBlk(SemBlk &&other); // 转移所有权
    explicit operator int(); // 获取存储的信号量id
    ~SemBlk(); // 释放信号量
};
```
## 数据操作
- writebuf获取共享内存和信号量，向内存中写数据，每次写一个Block，在read的字节数少于BUFSIZE时此块的end标识设为true，写入内存结束。
- readbuf向磁盘写数据，每次写一个Block，写完后检测当前Block的end标志，为true说明时最后一块，结束写入。

## 细节部分
创建新文件的mode从旧文件中使用lstat获取，保证文件的mode一致，可执行文件后期不需要手动加上可执行权限。
