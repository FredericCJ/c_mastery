#include "cursor.h"
int oc_rows_read(void *context,size_t row,size_t col,int *out)
{
    OcRows *r=context;
    if(!r || !out || !r->rows || row>=r->count || col>=3)return -1;
#ifdef OC_MUTANT_ROW
    *out=r->rows[0][col];
#else
    *out=r->rows[row][col];
#endif
    return 0;
}
int oc_table_read(void *context,size_t row,size_t col,int *out)
{
    OcTable *t=context;
    if(!t || !out || !t->rows || !t->lengths || row>=t->count || col>=t->lengths[row] || !t->rows[row])return -1;
    *out=t->rows[row][col];return 0;
}
int oc_read(const OcMatrix *m,size_t row,size_t col,int *out)
{
    if(!m || !m->read || !out || row>=m->rows || col>=m->columns)return -1;
    return m->read(m->context,row,col,out);
}
int oc_representation(int (*matrix)[2][3],unsigned char *out,size_t capacity)
{
    const unsigned char *bytes;
    if(!matrix || !out || capacity!=sizeof(*matrix))return -1;
    bytes=(const unsigned char *)matrix;
    for(size_t i=0;i<sizeof(*matrix);++i)out[i]=bytes[i];
    return 0;
}
FDStatus oc_take(FDCursor *c,size_t n,const unsigned char **out){return fd_take(c,n,out);}
FDStatus oc_subview(const FDCursor *c,size_t offset,size_t n,FDCursor *out){return fd_subview(c,offset,n,out);}
FDStatus oc_previous(FDCursor *c,size_t n,const unsigned char **out)
{
    size_t next;
    if(!c || !out || c->position>c->length || (!c->base && c->length))return FD_INVALID;
    if(n>c->position){
#ifdef OC_MUTANT_REVERSE
        c->position=0;
#endif
        return FD_RANGE;
    }
    next=c->position-n;
    *out=c->base?c->base+next:NULL;c->position=next;return FD_OK;
}
int oc_difference(const unsigned char *base,size_t n,size_t left,size_t right,ptrdiff_t *out)
{
    size_t magnitude;
    if(!out || !base || left>n || right>n)return -1;
    magnitude=left>right?left-right:right-left;
    if(magnitude>(size_t)PTRDIFF_MAX)return -1;
    *out=(base+left)-(base+right);return 0;
}
static int position(const size_t *lengths,size_t count,OcPosition p,size_t *out)
{
    size_t sum=0;
    if(!lengths || p.segment>=count || p.index>lengths[p.segment])return -1;
    for(size_t i=0;i<count;++i){if(lengths[i]>SIZE_MAX-sum)return -1;sum+=lengths[i];}
    sum=0;for(size_t i=0;i<p.segment;++i)sum+=lengths[i];*out=sum+p.index;return 0;
}
int oc_order(const size_t *lengths,size_t count,OcPosition a,OcPosition b,int *out)
{
    size_t x,y;
    if(!out || position(lengths,count,a,&x) || position(lengths,count,b,&y))return -1;
#ifdef OC_MUTANT_FRAGMENT
    x=a.index;y=b.index;
#endif
    *out=x<y?-1:x>y?1:0;return 0;
}
int oc_encode_index(int *base,size_t count,const int *p,unsigned char *out)
{
    uint32_t index=UINT32_MAX;size_t i;
    if(!out || (!base && count))return -1;
#if SIZE_MAX > UINT32_MAX
    if(count>UINT32_MAX)return -1;
#endif
    if(p){for(i=0;i<count;++i)if(p==&base[i])break;if(i==count)return -1;index=(uint32_t)i;}
    for(i=0;i<4;++i)out[i]=(unsigned char)(index>>(24-8*i));
    return 0;
}
int oc_decode_index(int *base,size_t count,const unsigned char *wire,int **out)
{
    uint32_t index=0;int *value;
    if(!wire || !out || (!base && count))return -1;
#if SIZE_MAX > UINT32_MAX
    if(count>UINT32_MAX)return -1;
#endif
    for(size_t i=0;i<4;++i)index=(index<<8)|wire[i];
#ifdef OC_MUTANT_NULL
    if(index==UINT32_MAX)return -1;
#else
    if(index==UINT32_MAX)value=NULL;
    else
#endif
    {if(index>=count)return -1;value=&base[index];}
    *out=value;return 0;
}
static int identity(int x){return x;}
int oc_roundtrip(int *p)
{
    void *untyped=p;int *again=untyped;int (*function)(int)=identity;
    void (*other)(void)=(void (*)(void))function;
    int (*restored)(int)=(int (*)(int))other;
    if(again!=p || restored!=function || restored(7)!=7)return -1;
    return 0;
}
