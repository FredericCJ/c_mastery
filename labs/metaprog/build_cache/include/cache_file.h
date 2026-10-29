#ifndef CMASTERY_CACHE_FILE_H
#define CMASTERY_CACHE_FILE_H
#include "build_cache.h"
enum { MPB_OPEN=1,MPB_WRITE,MPB_FILE_SYNC,MPB_CLOSE,MPB_RENAME,MPB_DIRECTORY_SYNC };
typedef int (*mpb_boundary_fn)(void *context,unsigned stage,size_t transferred);
int mpb_file_put(int directory,unsigned id,const unsigned char *bytes,size_t length,mpb_boundary_fn boundary,void *context);
int mpb_file_match(int directory,unsigned id,const unsigned char *expected,size_t length);
int mpb_file_recover(int directory,unsigned id);
#endif
