#ifndef CORPUS_QUERY_H
#define CORPUS_QUERY_H
#include <stddef.h>
#include <stdint.h>
enum { QI_OK, QI_INVALID, QI_SPACE };
enum { QI_ROWS=32, QI_TERMS=8, QI_TOKENS=8, QI_GROUP=4 };
typedef struct { uint8_t alive,has_value,length,token[8]; int16_t value; } QiRow;
typedef struct { uint8_t row,positions; } QiPosting;
typedef struct { int16_t minimum,maximum; uint8_t present,exact; } QiZone;
typedef struct {
    QiRow row[32]; size_t count;
    uint32_t live,term[8];
    QiPosting postings[8][32]; uint8_t posting_count[8];
    QiZone zone[8];
} QiIndex;
typedef struct { uint32_t yes,no; } QiTruth;
typedef struct { unsigned groups,rows; } QiCost;
/* Inputs are valid initialized indexes, with live disjoint output objects.
   Rebuild and replace stage all outputs; rejected calls leave outputs intact.
   Row numbers remain stable across deletion and replacement. */
int qi_build(const QiRow *rows,size_t n,QiIndex *out);
int qi_replace(QiIndex *index,size_t row,QiRow value,int lazy_zone);
int qi_rebuild_zones(QiIndex *index);
int qi_terms(const QiIndex *index,unsigned required,unsigned forbidden,uint32_t *out);
int qi_compare(const QiIndex *index,int16_t threshold,QiTruth *out);
int qi_logic(const QiIndex *index,QiTruth a,QiTruth b,unsigned operation,QiTruth *out);
int qi_phrase(const QiIndex *index,const uint8_t *query,size_t n,uint32_t *out);
int qi_near(const QiIndex *index,unsigned a,unsigned b,unsigned distance,uint32_t *out);
int qi_range(const QiIndex *index,int16_t low,int16_t high,uint32_t *out,QiCost *cost);
int qi_search(const QiIndex *index,unsigned required,unsigned forbidden,int16_t low,int16_t high,const uint8_t *phrase,size_t n,uint32_t *out,QiCost *cost);
#endif
