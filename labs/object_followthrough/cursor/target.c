#include "cursor.h"
static unsigned char bytes[4]={1,2,3,4},copy[sizeof(int[2][3])],wire[4];
static int values[2][3]={{1,2,3},{4,5,6}},out;
static const unsigned char *view;
static int *nullable;
static ptrdiff_t difference;
static size_t lengths[2]={3,3};
static int *table[2]={values[0],values[1]};
volatile int oc_target_result;
void _start(void)
{
    struct oc_rows rows={values,2};struct oc_table fragmented={table,lengths,2};
    struct oc_matrix m={&rows,2,3,oc_rows_read};struct fd_cursor c={bytes,4,0},sub;
    int result=oc_read(&m,1,2,&out);m.context=&fragmented;m.read=oc_table_read;
    result+=oc_read(&m,1,2,&out);result+=oc_representation(&values,copy,sizeof copy);
    result+=(int)oc_take(&c,1,&view);result+=(int)oc_subview(&c,0,2,&sub);
    result+=(int)oc_previous(&c,1,&view);result+=oc_difference(bytes,4,4,0,&difference);
    result+=oc_order(lengths,2,(struct oc_position){0,3},(struct oc_position){1,0},&out);
    result+=oc_encode_index(values[0],3,&values[0][2],wire);
    result+=oc_decode_index(values[0],3,wire,&nullable);result+=oc_roundtrip(nullable);
#ifdef OC_TARGET_HELPER
    {extern void *malloc(size_t);nullable=malloc(4);}
#endif
    oc_target_result=result;for(;;){}
}
