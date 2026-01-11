#include <linux/module.h>
#include <linux/kernel.h>


static char *word = "world";
module_param(word, charp, 0);
MODULE_PARM_DESC(word, "To print after hello");

static int count = 1;
module_param(count, int, 0);
MODULE_PARM_DESC(count, "To print the number of tiimes");

static int __init my_init(void){    
    for(int i = 0; i<count; i++){
        pr_info("%d Hello %s", i, word);
    }
    return 0;    
}


static void __exit my_exit(void){
    pr_info("Goodbye");    
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANx LALA");


