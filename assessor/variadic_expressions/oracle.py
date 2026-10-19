"""Independent exact text tables and arbitrary-precision range models."""
from itertools import product
import json
def generate(c):
 symbols=[('I',-2,'I=-2;'),('I',0,'I=0;'),('I',7,'I=7;'),('D',-0.5,'D=-0.50;'),('D',0.0,'D=0.00;'),('D',1.25,'D=1.25;')]
 cases=[seq for n in range(4) for seq in product(symbols,repeat=n)]
 chunks=['#include "format.h"\n#include "expressions.h"\n#include <limits.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n_Static_assert(INT_MAX==2147483647 && UCHAR_MAX==255,"oracle recorded native model");\nstatic char *get(void *p,size_t n){(void)p;return malloc(n);}\nstatic void put(void *p,char *v){(void)p;free(v);}\nstatic TfStatus call(unsigned id,char *out,size_t n,size_t *r,TfAllocator a,TfScalars s){switch(id){']
 for i,seq in enumerate(cases):
  args=','.join("'%s',%s"%(t,repr(v)) for t,v,_ in seq)
  chunks.append(f'case {i}:return tf_format(out,n,r,a,s,{len(seq)}u'+(','+args if args else '')+');')
 chunks.append('default:return TF_INVALID;}}\nstatic const char *expected[]={'+','.join(json.dumps(''.join(t for _,_,t in seq)) for seq in cases)+'};')
 lo,hi=-(2**31),2**31-1
 values=list(range(-8,9))+[lo,lo+1,hi-1,hi]
 rows=[]
 for a,b,d in product(values,repeat=3):
  v=b*d;total=a+v;ok=lo<=v<=hi and lo<=total<=hi
  rows.append([a,b,d,int(ok),total if ok else 0])
 c.write('independent-equations.json',json.dumps(dict(format_cases=[dict(pairs=[(t,v) for t,v,_ in s],text=''.join(x for _,_,x in s)) for s in cases],integer_model=dict(minimum=lo,maximum=hi,char_max=255),arithmetic=rows),indent=2)+'\n')
 def literal(n):return 'INT_MIN' if n==lo else str(n)
 chunks.append('static const struct {int a,b,c,ok,result;} rows[]={'+','.join('{'+','.join(literal(n) for n in r)+'}' for r in rows)+'};')
 chunks.append('static TfStatus sa(void *p,int *v){*v=((int*)p)[0];return TF_OK;}\nstatic TfStatus sb(void *p,int *v){*v=((int*)p)[1];return TF_OK;}\nstatic TfStatus sc(void *p,int *v){*v=((int*)p)[2];return TF_OK;}\nstatic TfStatus gate(void *p){(void)p;return TF_OK;}')
 chunks.append('''int main(void){
 TfAllocator a={NULL,get,put};TfScalars s={NULL,tf_default_integer,tf_default_real};
 for(unsigned i=0;i<sizeof expected/sizeof expected[0];++i){
  size_t want=strlen(expected[i]);
  for(size_t cap=0;cap<=want+1;++cap){char out[64];memset(out,'@',sizeof out);size_t r=999;
   TfStatus status=call(i,cap?out:NULL,cap,&r,a,s);
   if(r!=want || status!=(cap<=want?TF_FULL:TF_OK)) return 1;
   if(status==TF_FULL){for(size_t j=0;j<sizeof out;++j)if(out[j]!='@')return 2;}
   else if(strcmp(out,expected[i]) || out[want+1]!='@')return 3;
  }
 }
 for(unsigned i=0;i<sizeof rows/sizeof rows[0];++i){int v[]={rows[i].a,rows[i].b,rows[i].c};TfProgress p={99,123};
  TfStatus status=tf_steps(sa,sb,sc,gate,v,&p);
  if(status!=(rows[i].ok?TF_OK:TF_SIZE)||p.completed!=3||p.result!=(rows[i].ok?rows[i].result:123))return 4;
 }
 for(long long v=-1024;v<=1024;++v){unsigned char out=77;int ok=v>=0&&v<=255;
  if(tf_narrow_signed(v,&out)!=(ok?TF_OK:TF_SIZE)||out!=(ok?v:77))return 5;
 }
''')
 chunks.append(f'puts("TF_ORACLE_PASS text={len(cases)} equations={len(rows)} narrow=2049");return 0;}}')
 return c.write('independent-oracle.c','\n'.join(chunks)+'\n'),dict(text=len(cases),equations=len(rows),narrow=2049)

def models(c):
 # Explicit hypothetical ranges, not compiler emulation. Conversion follows representability.
 rows=[]
 for int_bits in [16,32,64]:
  imax=2**(int_bits-1)-1;umax=2**int_bits-1
  short_max=65535;promoted='int' if short_max<=imax else 'unsigned int'
  value=65535+(-1 if promoted=='int' else umax)
  if promoted!='int':value%=umax+1
  rows.append(dict(int_bits=int_bits,unsigned_short_bits=16,promoted=promoted,short65535_plus_negative1=value,result_type=promoted,negative1_less_short1=promoted=='int',negative1_less_1u=False,conditional_true=umax))
 c.require([r['promoted'] for r in rows]==['unsigned int','int','int'],'promotion range models')
 c.require([r['conditional_true'] for r in rows]==[65535,4294967295,18446744073709551615],'conditional common unsigned type')
 c.require([r['negative1_less_short1'] for r in rows]==[False,True,True],'same spelling changed comparison')
 # In 32-bit int: int32_t=int, uint32_t=unsigned int. In hypothetical 64-bit int:
 # exact-width32 types are lower-rank short/unsigned short and BOTH promote to int.
 rows.extend([dict(model='32-bit int,32-bit int32_t and uint32_t',common='unsigned int',negative1_less_uint32_1=False),dict(model='64-bit int,32-bit short/unsigned short exact-width types',common='int',negative1_less_uint32_1=True)])
 c.write('promotion-models.json',json.dumps(rows,indent=2)+'\n');return rows
