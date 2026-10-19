#ifndef CMK2_H
#define CMK2_H
#include <stddef.h>
#include <stdint.h>
#define CMK2_MAX_CPUS 4u
#define CMK2_THREADS_PER_CPU 2u
#define CMK2_MAX_THREADS (CMK2_MAX_CPUS * CMK2_THREADS_PER_CPU)
#define CMK2_QUEUE_CAPACITY 8u
#define CMK2_MESSAGES_PER_CPU 256u
/* ABI v2 supplements, and does not renumber, cmk_abi.h v1. These bounded
 * kernel-object contracts are not a POSIX syscall ABI. */
enum cmk2_status { CMK2_OK = 0, CMK2_BLOCKED = 1, CMK2_FULL = -1,
  CMK2_EMPTY = -2, CMK2_INVALID = -3, CMK2_CLOSED = -4 };
enum cmk2_thread_state { CMK2_UNUSED, CMK2_READY, CMK2_RUNNING,
  CMK2_WAIT_SEND, CMK2_WAIT_RECV, CMK2_ZOMBIE };
struct cmk2_topology { uint32_t lapic; unsigned count; uint8_t ids[CMK2_MAX_CPUS]; };
struct cmk2_message { uint32_t sequence, value; };
struct cmk2_channel {
  struct cmk2_message ring[CMK2_QUEUE_CAPACITY];
  unsigned head, count, closed;
};
/* Caller holds the object's scheduler lock; copying and state transition
 * form one critical section in the guest. No internal sleep or allocation. */
int cmk2_channel_send(struct cmk2_channel *, struct cmk2_message);
int cmk2_channel_receive(struct cmk2_channel *, struct cmk2_message *);
void cmk2_channel_close(struct cmk2_channel *);
int cmk2_madt_parse(const void *, size_t, struct cmk2_topology *);
void cmk2_smp_run(unsigned multiboot_info);
void cmk2_switch(uint64_t **old_stack, uint64_t *new_stack);
unsigned cmk2_cpu_index(void);
unsigned cmk2_cpu_count(void);
void cmk2_eoi(void);
void cmk2_wake_cpu(unsigned);
void cmk2_flush_remote(void);
void cmk2_start_user_aps(void (*entry)(unsigned));
#endif
