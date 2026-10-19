#ifndef CMASTERY_DOD_TEXT_H
#define CMASTERY_DOD_TEXT_H
#include "dod.h"
int d_rope_index(const DRope *,unsigned,size_t,unsigned char *);
int d_rope_split(DRope *,unsigned,size_t,unsigned *,unsigned *);
int d_rope_splice(DRope *,unsigned,size_t,size_t,const unsigned char *,size_t,unsigned *);
int d_rope_balance(DRope *,unsigned,unsigned *);
int d_utf8_offset(const unsigned char *,size_t,size_t,size_t *);
typedef struct {unsigned char bytes[128];size_t low,high;} DGap;
void d_gap_init(DGap *);
int d_gap_insert(DGap *,size_t,const unsigned char *,size_t);
int d_gap_delete(DGap *,size_t,size_t);
int d_gap_flatten(const DGap *,unsigned char *,size_t,size_t *);
int d_gap_scalar_edit(DGap *,size_t,size_t,const unsigned char *,size_t);
typedef struct {unsigned source;size_t offset,length;} DPiece;
typedef struct {const unsigned char *original;unsigned char *added;size_t original_length,add_length,add_capacity,length;DPiece pieces[32];unsigned count;} DPieceTable;
int d_piece_init(DPieceTable *,const unsigned char *,size_t,unsigned char *,size_t);
int d_piece_insert(DPieceTable *,size_t,const unsigned char *,size_t);
int d_piece_delete(DPieceTable *,size_t,size_t);
int d_piece_flatten(const DPieceTable *,unsigned char *,size_t,size_t *);
int d_piece_compact(const DPieceTable *,unsigned char *,size_t,unsigned char *,size_t,DPieceTable *);
int d_piece_scalar_edit(DPieceTable *,size_t,size_t,const unsigned char *,size_t);
#endif
