#include "cmk2.h"
/* Implement bounded AP discovery, bootstrap, local APIC, per-CPU state,
 * cooperative kernel threads and atomic sleep/wakeup under the scheduler lock. */
void cmk2_smp_run(unsigned info) { (void)info; }

unsigned cmk2_cpu_index(void){return 0;}
unsigned cmk2_cpu_count(void){return 1;}
void cmk2_eoi(void){}
void cmk2_wake_cpu(unsigned c){(void)c;}
void cmk2_flush_remote(void){}
void cmk2_start_user_aps(void (*entry)(unsigned)){(void)entry;}
