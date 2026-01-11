#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/module.h>

#define DEVICE_NAME "simple_driver"
#define BUF_LEN 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ansh Srivastav");
MODULE_DESCRIPTION("A simple character device driver made by Ansh Lala...");

static int msg_size;
static int major_num;
static char kernel_buffer[BUF_LEN];

static int device_open(struct inode *inode, struct file *filp) {
    pr_info("Simple Driver: Device Opened! with Minor: %d && Major: %d", iminor(inode), imajor(inode));

    pr_info("Simple Driver: filp->f_pos: %lld\n", filp->f_pos);
    pr_info("Simple Driver: filp->f_mode: 0x%x\n", filp->f_mode);
    pr_info("Simple Driver: filp->f_flags: 0x%x\n", filp->f_flags);
    
    return 0;
}

static int device_release(struct inode *inode, struct file *filp) {
    pr_info( "Simple Driver: Device Released!");
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset){
    int bytes_read = 0;
    if(*offset >= msg_size){
        return 0;
    }
    
    bytes_read = msg_size - *offset;
    if(bytes_read > len) {
        bytes_read = len;
    }

    if(copy_to_user(buffer, kernel_buffer + *offset, bytes_read)){
        return -EFAULT;
    }

    *offset += bytes_read;

    pr_info( "Simple Driver: Sent %d bytes to user && value of the len is %zu\n", bytes_read, len);
    return bytes_read;
}
 
static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset){ // here the len will be equal to the number of characters eneterd. Ex for "hello nigga", len will be 12.(\n will also be included)
    if (len > BUF_LEN-1){
        pr_info( "Simple Driver: Buffer Overflow Protection\n");
        return -EINVAL;
    }

    if(copy_from_user(kernel_buffer, buffer, len)){
        return -EFAULT;
    }

    kernel_buffer[len] = '\0';
    msg_size = len;

    pr_info( "Simple Driver: Received %zu bytes: %s\n", len, kernel_buffer);
    return len;
}

static struct file_operations fops={
    .owner = THIS_MODULE,
    .read = device_read,
    .release = device_release,
    .write = device_write,
    .open = device_open,
};

static int __init my_init(void){
    pr_info( "Hello Anx");
    major_num = register_chrdev(0, DEVICE_NAME, &fops);

    if(major_num < 0){
        pr_err("Failed to register the major number\n");
        return major_num;
    }

    pr_info("SIMPLE Driver: Registered Succesfully with major number as: %d\n", major_num);
    return 0;
}

static void __exit my_exit(void){
    unregister_chrdev(major_num, DEVICE_NAME);
    pr_info( "Simple Driver: Goodbye Anx");
}

module_init(my_init);
module_exit(my_exit);


