#ifndef CMASTERY_DOD_PERSISTENT_H
#define CMASTERY_DOD_PERSISTENT_H
#include "dod.h"
typedef struct {DTree *arena;unsigned root,focus,depth,crumb[3],direction[3];} DZipper;
int d_zip_open(DTree *,unsigned,DZipper *);
int d_zip_down(DZipper *,unsigned);
int d_zip_up(DZipper *);
int d_zip_write(const DZipper *,int,unsigned *);
int d_tree_census(const DTree *,const unsigned *,size_t,unsigned *,unsigned *);
int d_hamt_delete(DHamt *,unsigned,unsigned,unsigned *);

typedef struct {void *context;int (*clone)(void *,const unsigned char *,size_t,unsigned char **);void (*drop)(void *,unsigned char *);} DTextOps;
typedef struct {unsigned left,right,refs;unsigned char *text;size_t length;} DRcNode;
typedef struct {DRcNode nodes[65];DTextOps ops;unsigned live,attempts,fail_at,reclaimed;} DRcTree;
typedef struct {DRcTree *owner;unsigned pending[65],count;} DRcRetire;
void d_rc_init(DRcTree *,DTextOps);
int d_rc_retain(DRcTree *,unsigned);
int d_rc_release(DRcTree *,unsigned);
int d_rc_update(DRcTree *,unsigned,unsigned,const unsigned char *,size_t,unsigned *);
int d_rc_get(const DRcTree *,unsigned,unsigned,const unsigned char **,size_t *);
int d_rc_retire(DRcTree *,unsigned,DRcRetire *);
int d_rc_drain(DRcRetire *,unsigned budget,unsigned *done);
#endif
