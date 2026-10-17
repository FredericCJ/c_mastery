#ifndef CMASTERY_B05_H
#define CMASTERY_B05_H
#include <stddef.h>
#include <stdint.h>
enum { B5_OK=0,B5_INVALID=1,B5_RANGE=2,B5_CONFLICT=3,B5_PENDING=4,B5_UNIMPLEMENTED=9 };
/* All are original bounded single-thread state models, not OS/hardware APIs. */
enum { B5_PUSH=0,B5_POP=1,B5_ADD=2,B5_JUMP=3,B5_BRANCH=4,B5_HALT=5 };
typedef struct { unsigned opcode,target; } B5Instruction;
int b5_verify(const B5Instruction *program,size_t length,unsigned *maximum);
typedef struct { uint32_t generation,value,checksum; unsigned commit; } B5Redo;
uint32_t b5_checksum(uint32_t generation,uint32_t value);
int b5_recover(uint32_t checkpoint_generation,uint32_t checkpoint_value,
                const B5Redo *records,size_t count,uint32_t *value);
typedef struct { unsigned present,user,writable; unsigned char bytes[16]; } B5Page;
int b5_copy_to_user(B5Page pages[4],size_t address,const unsigned char *input,size_t count);
typedef struct { uint32_t last,remaining; unsigned armed,expired; } B5Timer;
int b5_timer_arm(B5Timer *timer,uint32_t now,uint32_t delay);
int b5_timer_poll(B5Timer *timer,uint32_t now,int *expired);
typedef struct { uint32_t cost,period,jitter; } B5Task;
/* Fixed-priority higher-priority tasks supplied explicitly, max4. */
int b5_response(uint32_t own_cost,uint32_t blocking,uint32_t deadline,
                 const B5Task *higher,size_t count,uint32_t *response);
typedef struct { uint32_t next_ticket,active[4],waiting[4]; unsigned pending; } B5Grace;
int b5_read_enter(B5Grace *g,unsigned reader,uint32_t *ticket);
int b5_read_exit(B5Grace *g,unsigned reader,uint32_t ticket);
int b5_grace_begin(B5Grace *g);
int b5_grace_done(B5Grace *g,int *done);
typedef struct { uint32_t generation; unsigned state,result,terminal_count; } B5Request;
enum { B5_IDLE=0,B5_INFLIGHT=1,B5_CANCEL_REQUESTED=2,B5_QUIESCENT=3,B5_RETIRED=4 };
enum { B5_SUBMIT=0,B5_CANCEL=1,B5_COMPLETE=2,B5_REAP=3 };
int b5_request(B5Request *r,unsigned operation,uint32_t generation,unsigned result);
#endif
