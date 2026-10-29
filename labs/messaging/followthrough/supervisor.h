#ifndef CMASTERY_MESSAGING_SUPERVISOR_FOLLOW_H
#define CMASTERY_MESSAGING_SUPERVISOR_FOLLOW_H
#include "owned.h"
typedef bool (*mfs_open_fn)(void *,unsigned,unsigned);
typedef void (*mfs_close_fn)(void *,unsigned,unsigned);
struct mfs_tree {
  struct msg_supervisor parent, children[4];
  struct mfa_actor actors[4][4];
  bool resources[4][4], closed;
  mfs_open_fn open;
  mfs_close_fn close;
  void *resource_ctx;
  size_t child_limit;
  uint64_t window,last;
};
struct mfs_report {
  int events[64]; size_t count;
  unsigned attempts;
  bool parent_used, rollback, closed;
};
enum msg_status mfs_init(struct mfs_tree *,size_t,size_t,uint64_t,mfs_open_fn,mfs_close_fn,void *,
                   mfa_handler_fn,void *[4][4],mfa_dead_letter_fn,void *);
/* failure_mask bits0..7 select the actual constructor attempts within this call. */
enum msg_status mfs_fault(struct mfs_tree *,unsigned,unsigned,uint64_t,enum msg_strategy,unsigned,struct mfs_report *);
enum msg_status mfs_destroy(struct mfs_tree *);
#endif
