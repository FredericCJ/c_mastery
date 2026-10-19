#ifndef PC_IO_H
#define PC_IO_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
enum PCStatus { PC_DONE, PC_YIELD, PC_WAIT_READ, PC_WAIT_WRITE, PC_ERROR };
typedef struct { unsigned char bytes[64]; size_t next,end; uintmax_t committed; int error; bool eof; } PCCopy;
typedef struct {
    void *context;
    ssize_t (*read)(void *,unsigned char *,size_t);
    ssize_t (*write)(void *,const unsigned char *,size_t);
} PCOps;
void pc_init(PCCopy *copy);
enum PCStatus pc_resume(PCCopy *copy,PCOps ops,unsigned call_budget);
uintmax_t pc_off_max(void);
int pc_extent(uintmax_t offset,uintmax_t length,uintmax_t file_size,off_t *converted);
/* All descriptors borrowed; reject Linux O_APPEND before positioned writes.
 * Positive errno and exact committed prefix returned; no implicit durability. */
int pc_write_record(int fd,uintmax_t offset,const unsigned char *data,size_t length,uintmax_t file_size,size_t *committed);
enum PCClosePolicy { PC_LINUX_RELEASES, PC_EINTR_RETAINS };
/* Pure transition model. Retains-policy leaves owned=true on EINTR; Linux
 * consumes ownership even on EINTR. error/result is always returned separately. */
int pc_close_transition(bool *owned,int error,enum PCClosePolicy policy);
#endif
