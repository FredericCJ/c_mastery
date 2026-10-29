#ifndef PT_TERMINAL_H
#define PT_TERMINAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <termios.h>
#include <sys/types.h>
/* No controlling terminal. A guard borrows only the caller's newly owned PTY. */
struct pt_guard { int fd; struct termios saved; bool saved_valid,restore_pending; };
enum pt_mode { PT_CANONICAL,PT_RAW };
void pt_guard_init(struct pt_guard *guard);
int pt_enter(struct pt_guard *guard,int fd,enum pt_mode mode,unsigned minimum,unsigned deciseconds);
int pt_restore(struct pt_guard *guard);
bool pt_attributes_equal(const struct termios *a,const struct termios *b);
struct pt_pipeline { pid_t children[3]; unsigned reaped_mask; int status[3]; size_t bytes;uint32_t sum; };
/* Original producer -> copier -> sink; owned children only. early_sink causes
 * sink closure after a bounded prefix. Every created child is reaped before
 * return, including acquisition/deadline failure. Signal policy changes occur
 * only in children. No external executable is launched. */
int pt_pipeline(size_t bytes,bool early_sink,unsigned deadline_ms,struct pt_pipeline *out);
#endif
