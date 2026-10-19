#ifndef SS_SECRETS_H
#define SS_SECRETS_H
#include <stddef.h>
typedef struct { unsigned equal, work; } SSCompare;
typedef struct { unsigned char bytes[4]; unsigned live; } SSPool;
int ss_compare(const unsigned char token[4], const unsigned char *candidate,
               size_t length, int early, SSCompare *out);
int ss_indexed(const unsigned char token[4], const unsigned char candidate[4],
               volatile const unsigned char table[256], unsigned trace[8]);
int ss_acquire(SSPool *pool, const unsigned char token[4]);
int ss_release(SSPool *pool);
void ss_wipe(void *p, size_t count);
unsigned ss_consume(const unsigned char token[4]);
unsigned ss_dead_store(const unsigned char token[4]);
unsigned ss_explicit_store(const unsigned char token[4]);
#endif
