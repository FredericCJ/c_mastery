#ifndef OC_CURSOR_H
#define OC_CURSOR_H
#include "depth.h"
typedef struct { int (*rows)[3]; size_t count; } OcRows;
typedef struct { int **rows; const size_t *lengths; size_t count; } OcTable;
typedef struct { void *context; size_t rows,columns; int (*read)(void *,size_t,size_t,int *); } OcMatrix;
typedef struct { size_t segment,index; } OcPosition;
int oc_rows_read(void *,size_t,size_t,int *);
int oc_table_read(void *,size_t,size_t,int *);
int oc_read(const OcMatrix *,size_t,size_t,int *);
int oc_representation(int (*)[2][3],unsigned char *,size_t);
FDStatus oc_take(FDCursor *,size_t,const unsigned char **);
FDStatus oc_subview(const FDCursor *,size_t,size_t,FDCursor *);
FDStatus oc_previous(FDCursor *,size_t,const unsigned char **);
int oc_difference(const unsigned char *,size_t,size_t,size_t,ptrdiff_t *);
int oc_order(const size_t *,size_t,OcPosition,OcPosition,int *);
int oc_encode_index(int *,size_t,const int *,unsigned char *);
int oc_decode_index(int *,size_t,const unsigned char *,int **);
int oc_roundtrip(int *);
#endif
