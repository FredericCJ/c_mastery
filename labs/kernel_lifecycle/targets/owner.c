#include <linux/module.h>
#include <linux/init.h>
static int __init owner_start(void) { return -ENOSYS; /* learner lifecycle */ }
static void __exit owner_stop(void) { }
module_init(owner_start); module_exit(owner_stop); MODULE_LICENSE("GPL");
