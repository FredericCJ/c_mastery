#ifndef CMASTERY_RESILIENCE_JOURNAL_POSIX_H
#define CMASTERY_RESILIENCE_JOURNAL_POSIX_H
#include "journal.h"
typedef struct {int descriptor;size_t max_io;} RzjPosix;
int rzj_posix(int,size_t,RzjPosix *,RzjIO *);
#endif
