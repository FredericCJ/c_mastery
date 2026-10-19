// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
static int __init start(void){pr_info("AK_WORK_STUB unimplemented_admission_and_drain\n");return -EOPNOTSUPP;}
static void __exit stop(void){}
module_init(start);module_exit(stop);MODULE_LICENSE("GPL");
