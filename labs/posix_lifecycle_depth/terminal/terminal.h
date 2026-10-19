#ifndef PT_TERMINAL_H
#define PT_TERMINAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <termios.h>
#include <sys/types.h>
/* No controlling terminal. A guard borrows only the caller's newly owned PTY. */
typedef struct { int fd; struct termios saved; bool saved_valid,restore_pending; } PTGuard;
enum PTMode { PT_CANONICAL,PT_RAW };
void pt_guard_init(PTGuard *guard);
int pt_enter(PTGuard *guard,int fd,enum PTMode mode,unsigned minimum,unsigned deciseconds);
int pt_restore(PTGuard *guard);
bool pt_attributes_equal(const struct termios *a,const struct termios *b);
typedef struct { pid_t children[3]; unsigned reaped_mask; int status[3]; size_t bytes;uint32_t sum; } PTPipeline;
/* Original producer -> copier -> sink; owned children only. early_sink causes
 * sink closure after a bounded prefix. Every created child is reaped before
 * return, including acquisition/deadline failure. Signal policy changes occur
 * only in children. No external executable is launched. */
int pt_pipeline(size_t bytes,bool early_sink,unsigned deadline_ms,PTPipeline *out);
#endif
