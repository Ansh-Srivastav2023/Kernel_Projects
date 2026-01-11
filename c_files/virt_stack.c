#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>

#define DEVICE_NAME "virtual_stack"
#define BUF_LEN 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anx");
MODULE_DESCRIPTION("A Simple Virtual Stack...");

struct node{
    int data;
    struct node* next;
};

struct node* head = NULL;
// struct node* tempn;      //uncomment for queue implementation

static int major_num;

static int device_open(struct inode* inode, struct file* filp){
    pr_info("Virtual Stack: Device Opened...\n");
    return 0;
}

static int device_release(struct inode* inode, struct file* filp){
    pr_info("Virtual Stack: Device Closed...\n");
    return 0;
}

static ssize_t device_read(struct file* filp, char __user *buffer, size_t len, loff_t *offset){
    struct node* temp;
    char temp_buf[32];
    int length;
    
    if(head == NULL){
        return 0;
    }
    
    temp = head;
    int data_to_send = temp->data;
    head = head->next;

    kfree(temp);

    length = sprintf(temp_buf, "%d\n", data_to_send);

    if(copy_to_user(buffer, temp_buf, length)){
        return -EFAULT;
    }

    *offset = 0;

    pr_info("Virtual Stack: %d popped...\n", data_to_send);
    return length;
    
}

static ssize_t device_write(struct file* filp, const char __user *buffer, size_t len, loff_t *offset){
    int value;
    struct node* newnode;

    if(kstrtoint_from_user(buffer, len, 10, &value)){
        return -EINVAL; 
    }

    newnode = kmalloc(sizeof(struct node), GFP_KERNEL);
    newnode->data = value;
    newnode->next = NULL;
     
    // uncomment for queue implementation
    // if(head == NULL){
    //     head = newnode;
    //     tempn = head;
    // }
    // else {
    //     tempn->next = newnode;
    //     tempn = tempn->next;
    // }

    newnode->next = head;
    head = newnode;

    pr_info("Virtual Stack: %d pushed...\n", value);
    return len;

}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .release = device_release,
    .open = device_open
};

static int __init my_init(void){
    major_num = register_chrdev(0, DEVICE_NAME, &fops);

    if(major_num < 0){
        pr_info("Virtual Stack: Failed to register the device...\n");
        return -EFAULT;
    }

    pr_info("Virtual Stack: Successfully registered the device number %d\n", major_num);
    return 0;
}

static void __exit my_exit(void){
    struct node *temp, *next;

    // Free all remaining nodes
    temp = head;
    while(temp != NULL){
        next = temp->next;
        kfree(temp);
        temp = next;
    }
    unregister_chrdev(major_num, DEVICE_NAME);
    pr_info("Virtual Stack: Successfully registered...\n");
}

module_init(my_init);
module_exit(my_exit);