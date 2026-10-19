#include "mutable_views.h"
#ifndef ADV_MUTATION
#define ADV_MUTATION 0
#endif
static size_t gcd(size_t a,size_t b){while(b){size_t next=a%b;a=b;b=next;}return a;}
AdvStatus adv_geometry(size_t cap,size_t rows,size_t cols,size_t rs,size_t cs,AdvGeometry *out){
 if(!out)return ADV_INVALID;
 AdvGeometry next={0,1};
 if(rows&&cols){
  if(!rs||!cs)return ADV_INVALID;
  if(rows-1>SIZE_MAX/rs||cols-1>SIZE_MAX/cs)return ADV_RANGE;
  size_t a=(rows-1)*rs,b=(cols-1)*cs;
  if(a>SIZE_MAX-b||a+b==SIZE_MAX)return ADV_RANGE;
  next.required=a+b+1;if(next.required>cap)return ADV_RANGE;
  size_t g=ADV_MUTATION==2?1:gcd(rs,cs);
  if(ADV_MUTATION!=1&&rows>cs/g&&cols>rs/g)next.injective=0;
 }
 *out=next;return ADV_OK;
}
AdvStatus adv_view(uint32_t *p,size_t cap,size_t rows,size_t cols,size_t rs,size_t cs,AdvView *out){
 if(!out)return ADV_INVALID;
 AdvGeometry geometry;AdvStatus status=adv_geometry(cap,rows,cols,rs,cs,&geometry);if(status!=ADV_OK)return status;
 ADView checked;if(!ad_view(p,cap,rows,cols,rs,cs,&checked))return ADV_INVALID;
 if(!geometry.injective)return ADV_ALIAS;
 *out=(AdvView){p,cap,rows,cols,rs,cs};return ADV_OK;
}
ADView adv_readonly(AdvView v){return(ADView){v.data,v.capacity,v.rows,v.columns,v.row_stride,v.column_stride};}
AdvStatus adv_set(AdvView v,size_t r,size_t c,uint32_t x){if(r>=v.rows||c>=v.columns)return ADV_RANGE;v.data[r*v.row_stride+c*v.column_stride]=x;return ADV_OK;}
AdvStatus adv_transpose(AdvView v,AdvView *out){if(!out)return ADV_INVALID;*out=(AdvView){v.data,v.capacity,v.columns,v.rows,ADV_MUTATION==3?v.row_stride:v.column_stride,ADV_MUTATION==3?v.column_stride:v.row_stride};return ADV_OK;}
AdvStatus adv_subview(AdvView v,size_t r,size_t c,size_t rows,size_t cols,AdvView *out){
 if(!out)return ADV_INVALID;
 if(r>v.rows||c>v.columns||rows>v.rows-r||cols>v.columns-c)return ADV_RANGE;
 AdvView next={v.data,v.capacity,rows,cols,v.row_stride,v.column_stride};
 if(rows&&cols){size_t offset=r*(ADV_MUTATION==4?v.columns:v.row_stride)+c*v.column_stride;next.data=v.data+offset;next.capacity=v.capacity-offset;}
 *out=next;return ADV_OK;
}
AdvStatus adv_reduce(ADView v,AdvMode mode,AdvReduction *out){
 if(!out||(mode!=ADV_SCALAR&&mode!=ADV_VECTOR&&mode!=ADV_AUTO))return ADV_INVALID;
 if(v.rows&&v.columns>SIZE_MAX/v.rows)return ADV_RANGE;
 if(mode==ADV_VECTOR&&v.rows&&v.columns&&v.column_stride!=1)return ADV_LAYOUT;
 AdvReduction next={0,mode==ADV_AUTO?(v.column_stride==1?ADV_VECTOR:ADV_SCALAR):mode,0,0};
 if(!v.rows||!v.columns){*out=next;return ADV_OK;}
 for(size_t r=0;r<v.rows;++r){size_t c=0;
  if(next.backend==ADV_VECTOR){for(;v.columns-c>=4;c+=4){uint64_t block=adv_vector4(v.data+r*v.row_stride+c);if(ADV_MUTATION!=7&&block>UINT32_MAX-next.sum)return ADV_OVERFLOW;next.sum+=(uint32_t)block;++next.vector_blocks;}}
  for(;c<v.columns;++c){if(ADV_MUTATION==8&&next.backend==ADV_VECTOR)break;uint32_t x=v.data[r*v.row_stride+c*v.column_stride];if(ADV_MUTATION!=7&&x>UINT32_MAX-next.sum)return ADV_OVERFLOW;next.sum+=x;++next.scalar_elements;}
 }
 *out=next;return ADV_OK;
}
