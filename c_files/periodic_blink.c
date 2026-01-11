#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#define DEVICE_NAME "periodic_blink"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ansh");

static int major_num;
static struct mutex lock;
static struct task_struct *blink_thread;
static int frequency = 0;

static int device_open(struct inode *inode, struct file *file) {
    pr_info("Blinker: The device is opened...\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    pr_info("Blinker: The device is released...\n");
    return 0;
}

static int blink_thread_func(void *data) {
    while (!kthread_should_stop()) {
        mutex_lock(&lock);
        int bl_freq = frequency;
        mutex_unlock(&lock);

        if (bl_freq > 0) {
            pr_info("Heartbeat Pulse\n");
            msleep(1000 / bl_freq);
        } else {
            msleep(100);
        }
    }
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset) {
    int value;

    if (kstrtoint_from_user(buffer, len, 10, &value))
        return -EINVAL;

    if (value < 0) {
        pr_info("Frequency can't be less than 0...\n");
        return -EINVAL;
    }

    mutex_lock(&lock);
    frequency = value;
    mutex_unlock(&lock);
    
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int __init my_init(void) {
    // mutex_init(&lock);
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        pr_info("Failed to register the major number...\n");
        return major_num;
    }

    blink_thread = kthread_run(blink_thread_func, NULL, "blink_thread");
    if (IS_ERR(blink_thread)) {
        pr_info("Failed to create thread...\n");
        unregister_chrdev(major_num, DEVICE_NAME);
        return PTR_ERR(blink_thread);
    }

    pr_info("Blinker: Registered the device number = %d\n", major_num);
    return 0;
}

static void __exit my_exit(void) {
    if (blink_thread) {
        kthread_stop(blink_thread);
    }
    unregister_chrdev(major_num, DEVICE_NAME);
    pr_info("Blinker: Unregistered the device number...\n");
}

module_init(my_init);
module_exit(my_exit);