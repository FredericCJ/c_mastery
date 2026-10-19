#ifndef PM_MAP_H
#define PM_MAP_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
typedef struct {unsigned char *base;size_t mapped,logical;off_t offset;dev_t device;ino_t inode;uint64_t generation;int active;} PMMap;
typedef struct {size_t offset,length;uint64_t generation;} PMBorrow;
typedef struct {uint64_t session;size_t length;unsigned char payload[64];} PMRecord;
typedef void (*PMCut)(unsigned,void *);
int pm_map(int,off_t,size_t,size_t,PMMap *);
int pm_remap(PMMap *,int,size_t,size_t);
int pm_copy(const PMMap *,PMBorrow,void *,size_t);
int pm_unmap(PMMap *);
int pm_encode(unsigned char[128],const PMRecord *,unsigned);
int pm_decode(const unsigned char *,size_t,PMRecord *);
int pm_publish(int,uint64_t,const void *,size_t,PMCut,void *);
int pm_read_record(int,uint64_t,PMRecord *);
int pm_replace(int,uint64_t,uint64_t,const void *,size_t);
#endif
