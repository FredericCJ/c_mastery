/* Learner implementation required: allocation admission and fault classification. */
static uint32_t sf_fault_take(void){return 0;}
static unsigned sf_fault_class(uint64_t e,uint64_t p){(void)e;(void)p;return 0;}
static void sf_fault_before_lock(struct frame *f){(void)f;}
