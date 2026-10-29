#ifndef CORPUS_QUERY_H
#define CORPUS_QUERY_H
#include <stddef.h>
#include <stdint.h>
enum { QI_OK, QI_INVALID, QI_SPACE };
enum { QI_ROWS=32, QI_TERMS=8, QI_TOKENS=8, QI_GROUP=4 };
struct qi_row { uint8_t alive,has_value,length,token[8]; int16_t value; };
struct qi_posting { uint8_t row,positions; };
struct qi_zone { int16_t minimum,maximum; uint8_t present,exact; };
struct qi_index {
    struct qi_row row[32]; size_t count;
    uint32_t live,term[8];
    struct qi_posting postings[8][32]; uint8_t posting_count[8];
    struct qi_zone zone[8];
};
struct qi_truth { uint32_t yes,no; };
struct qi_cost { unsigned groups,rows; };
/* Inputs are valid initialized indexes, with live disjoint output objects.
   Rebuild and replace stage all outputs; rejected calls leave outputs intact.
   Row numbers remain stable across deletion and replacement. */
int qi_build(const struct qi_row *rows,size_t n,struct qi_index *out);
int qi_replace(struct qi_index *index,size_t row,struct qi_row value,int lazy_zone);
int qi_rebuild_zones(struct qi_index *index);
int qi_terms(const struct qi_index *index,unsigned required,unsigned forbidden,uint32_t *out);
int qi_compare(const struct qi_index *index,int16_t threshold,struct qi_truth *out);
int qi_logic(const struct qi_index *index,struct qi_truth a,struct qi_truth b,unsigned operation,struct qi_truth *out);
int qi_phrase(const struct qi_index *index,const uint8_t *query,size_t n,uint32_t *out);
int qi_near(const struct qi_index *index,unsigned a,unsigned b,unsigned distance,uint32_t *out);
int qi_range(const struct qi_index *index,int16_t low,int16_t high,uint32_t *out,struct qi_cost *cost);
int qi_search(const struct qi_index *index,unsigned required,unsigned forbidden,int16_t low,int16_t high,const uint8_t *phrase,size_t n,uint32_t *out,struct qi_cost *cost);
#endif
