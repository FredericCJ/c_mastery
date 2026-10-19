#include "abi.h"
/* Deliberately unsolved adapter. The model and ring implementation are separate
 * learner artifacts. Supply all callbacks required by the checked import hooks. */
static unsigned async_completions,async_wait_blocks;
static void async_poll(void){(void)async_completions;}
static int64_t async_submit(uint64_t h,uint64_t p){(void)h;(void)p;return RQ_UNIMPLEMENTED;}
static int64_t async_wait(uint64_t t){(void)t;return RQ_UNIMPLEMENTED;}
static int64_t rqa_status(uint64_t t){(void)t;return RQ_UNIMPLEMENTED;}
static void rqa_process_exit(unsigned p){(void)p;}
static int64_t rqa_syscall(struct frame *f){(void)f;(void)async_wait_blocks;return RQ_UNIMPLEMENTED;}
