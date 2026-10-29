#ifndef CMASTERY_DOD_TEXT_H
#define CMASTERY_DOD_TEXT_H
#include "dod.h"
int d_rope_index(const struct d_rope *,unsigned,size_t,unsigned char *);
int d_rope_split(struct d_rope *,unsigned,size_t,unsigned *,unsigned *);
int d_rope_splice(struct d_rope *,unsigned,size_t,size_t,const unsigned char *,size_t,unsigned *);
int d_rope_balance(struct d_rope *,unsigned,unsigned *);
int d_utf8_offset(const unsigned char *,size_t,size_t,size_t *);
struct d_gap {unsigned char bytes[128];size_t low,high;};
void d_gap_init(struct d_gap *);
int d_gap_insert(struct d_gap *,size_t,const unsigned char *,size_t);
int d_gap_delete(struct d_gap *,size_t,size_t);
int d_gap_flatten(const struct d_gap *,unsigned char *,size_t,size_t *);
int d_gap_scalar_edit(struct d_gap *,size_t,size_t,const unsigned char *,size_t);
struct d_piece {unsigned source;size_t offset,length;};
struct d_piece_table {const unsigned char *original;unsigned char *added;size_t original_length,add_length,add_capacity,length;struct d_piece pieces[32];unsigned count;};
int d_piece_init(struct d_piece_table *,const unsigned char *,size_t,unsigned char *,size_t);
int d_piece_insert(struct d_piece_table *,size_t,const unsigned char *,size_t);
int d_piece_delete(struct d_piece_table *,size_t,size_t);
int d_piece_flatten(const struct d_piece_table *,unsigned char *,size_t,size_t *);
int d_piece_compact(const struct d_piece_table *,unsigned char *,size_t,unsigned char *,size_t,struct d_piece_table *);
int d_piece_scalar_edit(struct d_piece_table *,size_t,size_t,const unsigned char *,size_t);
#endif
