#include <linux/module.h>
static int __init start(void){pr_info("AG_RCU_STUB mechanism_unimplemented\n");return -EOPNOTSUPP;}
static void __exit stop(void){}
module_init(start);module_exit(stop);MODULE_LICENSE("GPL");
