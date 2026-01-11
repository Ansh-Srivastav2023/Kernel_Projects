#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/sched/signal.h> // For for_each_process macro
#include <linux/jiffies.h>

static int my_stats_show(struct seq_file *m, void *v) {
    struct task_struct *task;
    int count = 0;

    rcu_read_lock();
    for_each_process(task) {
        count++;
    }
    rcu_read_unlock();

    unsigned long uptime = jiffies_to_msecs(get_jiffies_64()) / 1000;

    seq_printf(m, "\n--- Ansh's Kernel Stats ---\n");
    seq_printf(m, "Total Running Processes: %d\n", count);
    seq_printf(m, "System Uptime:           %lu seconds\n", uptime);
    
    return 0;
}

static int my_stats_open(struct inode *inode, struct file *file) {
    return single_open(file, my_stats_show, NULL);
}

static const struct proc_ops my_fops = {
    .proc_open = my_stats_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init proc_stats_init(void) {
    if (!proc_create("kernel_stats", 0444, NULL, &my_fops)) {
        return -ENOMEM;
    }
    printk(KERN_INFO "proc_stats: Module loaded. See /proc/kernel_stats\n\n");
    return 0;
}

static void __exit proc_stats_exit(void) {
    remove_proc_entry("kernel_stats", NULL);
    printk(KERN_INFO "proc_stats: Module unloaded.\n");
}

module_init(proc_stats_init);
module_exit(proc_stats_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ansh");
MODULE_DESCRIPTION("A simple proc kernel stats file...");
