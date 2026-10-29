#ifndef CMASTERY_DOD_PERSISTENT_H
#define CMASTERY_DOD_PERSISTENT_H
#include "dod.h"
struct d_zipper {struct d_tree *arena;unsigned root,focus,depth,crumb[3],direction[3];};
int d_zip_open(struct d_tree *,unsigned,struct d_zipper *);
int d_zip_down(struct d_zipper *,unsigned);
int d_zip_up(struct d_zipper *);
int d_zip_write(const struct d_zipper *,int,unsigned *);
int d_tree_census(const struct d_tree *,const unsigned *,size_t,unsigned *,unsigned *);
int d_hamt_delete(struct d_hamt *,unsigned,unsigned,unsigned *);

struct d_text_ops {void *context;int (*clone)(void *,const unsigned char *,size_t,unsigned char **);void (*drop)(void *,unsigned char *);};
struct d_rc_node {unsigned left,right,refs;unsigned char *text;size_t length;};
struct d_rc_tree {struct d_rc_node nodes[65];struct d_text_ops ops;unsigned live,attempts,fail_at,reclaimed;};
struct d_rc_retire {struct d_rc_tree *owner;unsigned pending[65],count;};
void d_rc_init(struct d_rc_tree *,struct d_text_ops);
int d_rc_retain(struct d_rc_tree *,unsigned);
int d_rc_release(struct d_rc_tree *,unsigned);
int d_rc_update(struct d_rc_tree *,unsigned,unsigned,const unsigned char *,size_t,unsigned *);
int d_rc_get(const struct d_rc_tree *,unsigned,unsigned,const unsigned char **,size_t *);
int d_rc_retire(struct d_rc_tree *,unsigned,struct d_rc_retire *);
int d_rc_drain(struct d_rc_retire *,unsigned budget,unsigned *done);
#endif
