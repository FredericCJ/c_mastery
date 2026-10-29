#ifndef OC_CURSOR_H
#define OC_CURSOR_H
#include "depth.h"
struct oc_rows { int (*rows)[3]; size_t count; };
struct oc_table { int **rows; const size_t *lengths; size_t count; };
struct oc_matrix { void *context; size_t rows,columns; int (*read)(void *,size_t,size_t,int *); };
struct oc_position { size_t segment,index; };
int oc_rows_read(void *,size_t,size_t,int *);
int oc_table_read(void *,size_t,size_t,int *);
int oc_read(const struct oc_matrix *,size_t,size_t,int *);
int oc_representation(int (*)[2][3],unsigned char *,size_t);
enum fd_status oc_take(struct fd_cursor *,size_t,const unsigned char **);
enum fd_status oc_subview(const struct fd_cursor *,size_t,size_t,struct fd_cursor *);
enum fd_status oc_previous(struct fd_cursor *,size_t,const unsigned char **);
int oc_difference(const unsigned char *,size_t,size_t,size_t,ptrdiff_t *);
int oc_order(const size_t *,size_t,struct oc_position,struct oc_position,int *);
int oc_encode_index(int *,size_t,const int *,unsigned char *);
int oc_decode_index(int *,size_t,const unsigned char *,int **);
int oc_roundtrip(int *);
#endif
