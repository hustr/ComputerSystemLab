#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
// 4K字节空间
#define MAX_SIZE 40

// 当前有效起始位置，第一个无效数据位置，设计一个环状缓冲区
static int cur, unuse;

struct test_dev {
    struct cdev chr_dev;
    // 使用内存
    unsigned char mem[MAX_SIZE];
} *mydev;
static dev_t ndev;

// static struct cdev chr_dev;
// static dev_t ndev;

// open将会调用的函数
static int test_open(struct inode* nd, struct file* filp)
{	
	printk("test_open\n");
	// 只有一个设备，不需要考虑filp
	return 0;
}

static int test_release(struct inode *nd, struct file*filp) {
	//  啥都不干就行了
	printk("test_release process!");
	return 0;
}

// read会调用的函数，需要将数据从用户空间复制到内核空间,copy_from_user
static ssize_t test_read(struct file* filp, char __user* buf, size_t sz, loff_t* off)
{
	printk("test_read process!\n");
	// off如果是针对单个设备的驱动的话是没有什么作用的
	// 直接使用一个全局静态变量吧
	if (sz > (unuse - cur + MAX_SIZE) % MAX_SIZE) {
		// 给定的数据大小无效
		sz = (unuse - cur + MAX_SIZE) % MAX_SIZE;
	}
	// 拷贝数据到用户内存中，注意数据跨结尾情况
	if (sz == 0) {
		return 0;
	}
    printk("cur = %d, unuse = %d, sz = %d\n", cur, unuse, sz);
	if (cur + sz < MAX_SIZE) {
		//  正常情况
		if(copy_to_user(buf, mydev->mem + cur, sz)) {
			return -1;
		}
		cur += sz;
	} else {
		// 拷贝要跨过尾部
		if (copy_to_user(buf, mydev->mem + cur, MAX_SIZE - cur)) {
			return -1;
		}
        printk("MAX_SIZE - cur = %d\n", MAX_SIZE - cur);
		if (copy_to_user(buf + MAX_SIZE - cur, mydev->mem, sz - (MAX_SIZE - cur))) {
			return -1;
		}
        printk("sz - (MAX_SIZE - cur) = %d\n", sz - (MAX_SIZE - cur));
		cur = (cur + sz) % MAX_SIZE;
	}

    printk("cur = %d, unuse = %d, sz = %d\n", cur, unuse, sz);

	return sz;
}
 
// write函数，需要将数据从内核空间写到用户空间，copy_to_user
static ssize_t test_write(struct file*filp, const char __user *buf, size_t sz, loff_t* off) {
	printk("test_write process!\n");
	// 使用全局的界写
	// 需要预留1字节防止首尾相连
	if (sz > MAX_SIZE - (unuse - cur + MAX_SIZE) % MAX_SIZE - 1) {
		sz = MAX_SIZE - (unuse - cur + MAX_SIZE) % MAX_SIZE - 1;
	}
	if (sz == 0) {
		return 0;
	}
	//  判断写是否会跨尾部
    printk("cur = %d, unuse = %d, sz = %d\n", cur, unuse, sz);
	if (sz + unuse >= MAX_SIZE) {
		// 先写一部分，再写剩下的
		if (copy_from_user(mydev->mem + unuse, buf, MAX_SIZE - unuse)) {
			return -1;
		}
		if (copy_from_user(mydev->mem, buf + MAX_SIZE - unuse, sz - (MAX_SIZE - unuse))) {
			return -1;
		}
		unuse = (unuse + sz) % MAX_SIZE;
	} else {
		// 直接写即可
		if(copy_from_user(mydev->mem + unuse, buf, sz)) {
			return -1;
		}
		unuse += sz;
	}
    printk("cur = %d, unuse = %d, sz = %d\n", cur, unuse, sz);
	return sz;
}

struct file_operations chr_ops = {
	.owner = THIS_MODULE,
	.open = test_open,
	.read = test_read,
	.write = test_write,
	.release = test_release
};
 
// 注册设备
static int test_init(void)
{
	int ret;
	ret = alloc_chrdev_region(&ndev, 0, 1, "test");
	if (ret < 0) {
		return ret;
	}
	printk("test_init(): major = %d, minor = %d\n", MAJOR(ndev), MINOR(ndev));

	mydev = kzalloc(sizeof(struct test_dev), GFP_KERNEL);
	if (!mydev) {
		return -1;
	}
	//初始化并添加设备
	cdev_init(&mydev->chr_dev, &chr_ops);
	ret = cdev_add(&mydev->chr_dev, ndev, 1);
	// 添加失败
	if (ret < 0) {
		kfree(mydev);
		return ret;
	}
	// 初始化缓冲区的数据
	cur = unuse = 0;
    
	return 0;
}
// 卸载设备
static void test_exit(void)
{
	printk("test_exit process!\n");
	// cdev_del(&chr_dev);
	cdev_del(&mydev->chr_dev);
	unregister_chrdev_region(ndev, 1);
	kfree(mydev);
}
 
module_init(test_init);
module_exit(test_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wangyaning6166@gmail.com");
MODULE_DESCRIPTION("Linux virtual character device drive");
