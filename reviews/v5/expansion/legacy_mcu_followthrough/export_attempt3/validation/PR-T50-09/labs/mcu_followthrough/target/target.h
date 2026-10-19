#ifndef MF_TARGET_H
#define MF_TARGET_H
#include <stddef.h>
#include <stdint.h>
#ifndef MF_FP
#define MF_FP 0
#endif
typedef struct { uint32_t integer[8], high_fp[16], fpscr;
                 uint32_t observed_integer[8], observed_fp[16], observed_fpscr;
                 uint32_t rounded; } MfProbe;
void mf_direct_switch(uint32_t **old_sp, uint32_t *new_sp);
void mf_direct_probe(MfProbe *);
void mf_direct_enter(void);
void mf_direct_yield(void);
void mf_direct_return(unsigned task);
void mf_main(void);
void mf_trap(void);
void mf_text(const char *);
void mf_number(uint32_t);
void mf_finish(unsigned);
void mf_require(int, unsigned);
uint32_t mf_udiv32(uint32_t, uint32_t);
uint64_t mf_udiv64(uint64_t, uint64_t);
#endif
