#include "dod_text.h"
#ifndef D_TEXT_MUTANT
#define D_TEXT_MUTANT 0
#endif
int d_rope_index(const DRope *r,unsigned root,size_t index,unsigned char *out){
 if(!r||!out||root>r->used)return D_INVALID;
 if(!root||index>=r->nodes[root].length)return D_RANGE;
 while(r->nodes[root].left){unsigned left=r->nodes[root].left;size_t n=r->nodes[left].length;
  if(index<n)root=left;else{index-=n;root=r->nodes[root].right;}}
 *out=r->nodes[root].bytes[index];return D_OK;
}
static int split(DRope *r,unsigned root,size_t at,unsigned *left,unsigned *right){
 if(!root||!at){*left=0;*right=root;return D_OK;}
 DRopeNode node=r->nodes[root];
 if(at==node.length){*left=root;*right=0;return D_OK;}
 if(!node.left){int status=d_rope_leaf(r,node.bytes,at,left);if(status)return status;return d_rope_leaf(r,node.bytes+at,node.length-at,right);}
 size_t n=r->nodes[node.left].length;unsigned a,b;int status;
 if(at<n){status=split(r,node.left,at,&a,&b);if(status)return status;*left=a;return d_rope_concat(r,b,node.right,right);}
 status=split(r,node.right,at-n,&a,&b);if(status)return status;*right=b;return d_rope_concat(r,node.left,a,left);
}
int d_rope_split(DRope *r,unsigned root,size_t at,unsigned *left,unsigned *right){
 if(!r||!left||!right||left==right||root>r->used)return D_INVALID;
 if(at>(root?r->nodes[root].length:0))return D_RANGE;
 unsigned before=r->used,a=0,b=0;int status=split(r,root,at,&a,&b);
 if(status){if(D_TEXT_MUTANT!=1)r->used=before;return status;}
 if(D_TEXT_MUTANT==2){unsigned swap=a;a=b;b=swap;}
 *left=a;*right=b;return D_OK;
}
static int balanced(DRope *r,const unsigned char *p,size_t n,unsigned *out){
 if(n<=8)return d_rope_leaf(r,p,n,out);
 size_t middle=n/2;unsigned a,b;int status=balanced(r,p,middle,&a);if(status)return status;
 status=balanced(r,p+middle,n-middle,&b);if(status)return status;return d_rope_concat(r,a,b,out);
}
int d_rope_splice(DRope *r,unsigned root,size_t at,size_t remove,const unsigned char *p,size_t n,unsigned *out){
 if(!r||!out||root>r->used||(!p&&n))return D_INVALID;
 size_t length=root?r->nodes[root].length:0;
 if(at>length||remove>length-at)return D_RANGE;
 if(n>128-(length-remove))return D_FULL;
 if(!remove&&!n){*out=root;return D_OK;}
 unsigned before=r->used,left,right,deleted,tail,inserted,joined,result;
 int status=d_rope_split(r,root,at,&left,&right);
 if(status==D_OK)status=d_rope_split(r,right,remove,&deleted,&tail);
 if(D_TEXT_MUTANT==8&&n)--n;
 if(status==D_OK)status=balanced(r,p,n,&inserted);
 if(status==D_OK)status=d_rope_concat(r,left,inserted,&joined);
 if(status==D_OK)status=d_rope_concat(r,joined,tail,&result);
 if(status){if(D_TEXT_MUTANT!=7)r->used=before;return status;}
 *out=result;return D_OK;
}
int d_rope_balance(DRope *r,unsigned root,unsigned *out){
 if(!r||!out||root>r->used)return D_INVALID;
 unsigned char bytes[128];size_t n=0;int status=d_rope_flatten(r,root,bytes,128,&n);if(status)return status;
 unsigned before=r->used,result=0;status=balanced(r,bytes,n,&result);if(status){r->used=before;return status;}
 *out=result;return D_OK;
}
int d_utf8_offset(const unsigned char *p,size_t n,size_t scalar,size_t *out){
 if(!out||(!p&&n))return D_INVALID;
 size_t count=0,answer=0,pos=0;unsigned found=0;
 while(pos<n){if(count==scalar){answer=pos;found=1;}unsigned lead=p[pos],width;uint32_t value,min;
  if(lead<128){width=1;value=lead;min=0;}
  else if(lead>=0xc2&&lead<=0xdf){width=2;value=lead&31;min=128;}
  else if(lead>=0xe0&&lead<=0xef){width=3;value=lead&15;min=2048;}
  else if(lead>=0xf0&&lead<=0xf4){width=4;value=lead&7;min=65536;}
  else return D_INVALID;
  if(width>n-pos)return D_INVALID;
  for(unsigned i=1;i<width;++i){if((p[pos+i]&0xc0)!=0x80)return D_INVALID;value=(value<<6)|(p[pos+i]&63);}
  if((value<min&&D_TEXT_MUTANT!=5)||value>0x10ffff||((value>=0xd800&&value<=0xdfff)&&D_TEXT_MUTANT!=6))return D_INVALID;
  pos+=width;++count;
 }
 if(count==scalar){answer=n;found=1;}
 if(!found)return D_RANGE;
 *out=answer;return D_OK;
}
void d_gap_init(DGap *g){g->low=0;g->high=128;}
static void gap_move(DGap *g,size_t pos){
 while(g->low>pos){--g->low;--g->high;g->bytes[g->high]=g->bytes[g->low];}
 while(g->low<pos){g->bytes[g->low]=g->bytes[g->high];++g->low;++g->high;}
}
int d_gap_insert(DGap *g,size_t pos,const unsigned char *p,size_t n){
 if(!g||(!p&&n))return D_INVALID;
 size_t length=128-(g->high-g->low);if(pos>length)return D_RANGE;
 if(n>g->high-g->low)return D_FULL;
 gap_move(g,pos);for(size_t i=0;i<n;++i)g->bytes[g->low++]=p[i];return D_OK;
}
int d_gap_delete(DGap *g,size_t pos,size_t n){
 if(!g)return D_INVALID;
 size_t length=128-(g->high-g->low);if(pos>length||n>length-pos)return D_RANGE;
 gap_move(g,pos);if(D_TEXT_MUTANT!=3)g->high+=n;return D_OK;
}
int d_gap_flatten(const DGap *g,unsigned char *out,size_t cap,size_t *written){
 if(!g||!written||(!out&&cap))return D_INVALID;
 size_t n=128-(g->high-g->low);if(cap<n)return D_FULL;
 for(size_t i=0;i<g->low;++i)out[i]=g->bytes[i];
 for(size_t i=g->high;i<128;++i)out[g->low+i-g->high]=g->bytes[i];
 *written=n;return D_OK;
}
static int scalar_range(const unsigned char *bytes,size_t length,size_t first,size_t count,const unsigned char *insert,size_t n,size_t *start,size_t *end){
 if(count>SIZE_MAX-first)return D_RANGE;
 size_t ignored;int status=d_utf8_offset(insert,n,0,&ignored);if(status)return status;
 status=d_utf8_offset(bytes,length,first,start);if(status)return status;
 return d_utf8_offset(bytes,length,first+count,end);
}
int d_gap_scalar_edit(DGap *g,size_t first,size_t count,const unsigned char *p,size_t n){
 if(!g||(!p&&n))return D_INVALID;
 unsigned char bytes[128];size_t length=0,a,b;(void)d_gap_flatten(g,bytes,128,&length);
 int status=scalar_range(bytes,length,first,count,p,n,&a,&b);if(status)return status;
 DGap next=*g;status=d_gap_delete(&next,a,b-a);if(status)return status;
 status=d_gap_insert(&next,a,p,n);if(status)return status;*g=next;return D_OK;
}
int d_piece_init(DPieceTable *t,const unsigned char *original,size_t n,unsigned char *added,size_t cap){
 if(!t||(!original&&n)||(!added&&cap)||n>128||cap>128)return D_INVALID;
 DPieceTable value={0};value.original=original;value.original_length=n;value.added=added;value.add_capacity=cap;value.length=n;
 if(n){value.count=1;value.pieces[0]=(DPiece){0,0,n};}*t=value;return D_OK;
}
static int push(DPieceTable *t,DPiece p){
 if(!p.length)return D_OK;
 if(t->count){DPiece *last=&t->pieces[t->count-1];if(last->source==p.source&&last->offset+last->length==p.offset){last->length+=p.length;return D_OK;}}
 if(t->count==32)return D_FULL;
 t->pieces[t->count++]=p;return D_OK;
}
static int range(const DPieceTable *old,DPieceTable *next,size_t start,size_t end){
 size_t position=0;
 for(unsigned i=0;i<old->count;++i){DPiece p=old->pieces[i];size_t until=position+p.length;
  if(start<until&&end>position){size_t a=start>position?start:position,b=end<until?end:until;
   int status=push(next,(DPiece){p.source,p.offset+a-position,b-a});if(status)return status;}
  position=until;}
 return D_OK;
}
static int piece_edit(DPieceTable *t,size_t pos,size_t remove,const unsigned char *p,size_t n){
 if(!t||(!p&&n))return D_INVALID;
 if(pos>t->length||remove>t->length-pos)return D_RANGE;
 if(n>128-(t->length-remove)||n>t->add_capacity-t->add_length)return D_FULL;
 DPieceTable next=*t;next.count=0;
 int status=range(t,&next,0,pos);if(status)return status;
 status=push(&next,(DPiece){1,t->add_length,n});if(status)return status;
 status=range(t,&next,pos+remove,t->length);if(status)return status;
 for(size_t i=0;i<n;++i)t->added[t->add_length+i]=p[i];
 next.add_length+=n;next.length=t->length-remove+n;
 if(D_TEXT_MUTANT==4&&remove)next.length=t->length+n;
 *t=next;return D_OK;
}
int d_piece_insert(DPieceTable *t,size_t pos,const unsigned char *p,size_t n){return piece_edit(t,pos,0,p,n);}
int d_piece_delete(DPieceTable *t,size_t pos,size_t n){return piece_edit(t,pos,n,NULL,0);}
int d_piece_flatten(const DPieceTable *t,unsigned char *out,size_t cap,size_t *written){
 if(!t||!written||(!out&&cap))return D_INVALID;
 if(cap<t->length)return D_FULL;
 size_t at=0;for(unsigned i=0;i<t->count;++i){DPiece p=t->pieces[i];const unsigned char *source=p.source?t->added:t->original;
  for(size_t j=0;j<p.length;++j)out[at++]=source[p.offset+j];}
 *written=t->length;return D_OK;
}
int d_piece_compact(const DPieceTable *old,unsigned char *snapshot,size_t capacity,unsigned char *added,size_t add_cap,DPieceTable *out){
 if(!old||!out||old==out||(!snapshot&&capacity)||(!added&&add_cap)||add_cap>128)return D_INVALID;
 if(capacity<old->length)return D_FULL;
 size_t n=0;int status=d_piece_flatten(old,snapshot,capacity,&n);if(status)return status;
 return d_piece_init(out,snapshot,n,added,add_cap);
}
int d_piece_scalar_edit(DPieceTable *t,size_t first,size_t count,const unsigned char *p,size_t n){
 if(!t||(!p&&n))return D_INVALID;
 unsigned char bytes[128];size_t length=0,a,b;(void)d_piece_flatten(t,bytes,128,&length);
 int status=scalar_range(bytes,length,first,count,p,n,&a,&b);if(status)return status;
 return piece_edit(t,a,b-a,p,n);
}
