#ifndef CMASTERY_MATCH_TRANSFER_H
#define CMASTERY_MATCH_TRANSFER_H
#include "lookup.h"
enum {MT_BUSY=7,MT_CLOSED=8};
typedef struct {ALStream prefix;uint64_t pending[65];size_t count,next;unsigned state;} MTStream;
typedef struct {size_t attempted,accepted;} MTReport;
int mt_init(MTStream*,const unsigned char*,size_t);
int mt_feed(MTStream*,const unsigned char*,size_t,ALPublish,void*,MTReport*);
int mt_drain(MTStream*,ALPublish,void*,MTReport*);
int mt_tests(unsigned);
int mt_failure(const char*,unsigned);
void mt_observation(const char*,unsigned,unsigned);
#endif
