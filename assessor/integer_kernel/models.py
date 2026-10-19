"""Independent mathematical expected results, with explicit model premises."""
import json

def generate(c):
    models={
      'int16':dict(CHAR_BIT=8,UCHAR_MAX=255,INT_MAX=32767,UINT_MAX=65535,LONG_MAX=2147483647,representation='twos complement; no padding',promoted_type='int',add=256,complement=-256,shift='undefined:65280 is not representable as signed int',decimal100000='long',decimal65535='long',hexFFFF='unsigned int',ushort65535_promotes='unsigned int',ushort_plus1=0,ushort_complement=0,ushort_shift8=65280),
      'int32':dict(CHAR_BIT=8,UCHAR_MAX=255,INT_MAX=2147483647,UINT_MAX=4294967295,representation='twos complement; no padding',promoted_type='int',add=256,complement=-256,shift=65280,decimal100000='int',decimal65535='int',hexFFFF='int',ushort65535_promotes='int'),
      'char16':dict(CHAR_BIT=16,INT_MAX=2147483647,UINT_MAX=4294967295,cell_type='unsigned char; model alternative to native unsigned-int cells',logical_octets=[18,52,86,120],units_for_four_octets=4,storage_bits=64,native_api_storage='4*sizeof(unsigned int)*CHAR_BIT bits; not necessarily64',packed_units=[0x1234,0x5678],packed_storage_bits=32,stream_unit='one C character; not necessarily one protocol octet',host_endian='does not change explicit value shifts; raw multioctet character transport needs separate serialization contract'),
      'limits':'These are exact arithmetic/type-rule models. They are not observations of an int16 or CHAR_BIT16 compiler. All actual host and M4 observations are separate.'}
    for bits in [16,32]:
      m=models['int'+str(bits)];assert 255+1==m['add'] and -255-1==m['complement']
      assert (255*256>m['INT_MAX'])==(bits==16)
      assert ((65535+1)%65536)==models['int16']['ushort_plus1']
      assert ((65535*256)%65536)==models['int16']['ushort_shift8']
    assert [(0x12345678>>(24-8*i))&255 for i in range(4)]==models['char16']['logical_octets']
    c.write('integer-models.json',json.dumps(models,indent=2)+'\n')
    c.require(True,'hypothetical type rules evaluated with explicit premises; unsafe signed shift unexecuted')
    lo,hi=-(2**31),2**31-1
    values=[lo,lo+1,-1073741824,-65536,-46341,-46340,-2,-1,0,1,2,46340,46341,65535,65536,1073741823,hi-1,hi]
    def literal(n):return '(-2147483647-1)' if n==lo else str(n)
    rows=[]
    for a in values:
      for b in values:
        for op,exact in enumerate([a+b,a-b,a*b]):
          ok=lo<=exact<=hi
          rows.append('{%s,%s,%s,%s,%s}'%(literal(a),literal(b),op,int(ok),literal(exact) if ok else 917))
    division=[]
    for a in values:
      for b in values:
        for policy in range(3):
          ok=b!=0 and not(a==lo and b==-1)
          q=r=0
          if ok:
            if policy==0:q=(abs(a)//abs(b))*(-1 if (a<0)!=(b<0) else 1)
            elif policy==1:q=a//b
            else:r=a%abs(b);q=(a-r)//b
            r=a-q*b;ok=lo<=q<=hi and lo<=r<=hi
          division.append('{%s,%s,%s,%s,%s,%s}'%(literal(a),literal(b),policy,int(ok),literal(q) if ok else 917,literal(r) if ok else 919))
    source='''#include "integer.h"
#include <limits.h>
#include <stdio.h>
_Static_assert(INT_MIN==(-2147483647-1) && INT_MAX==2147483647,"recorded native int32 model");
typedef struct {int a,b,op,ok,value;} Row;
typedef struct {int a,b,policy,ok,q,r;} Division;
static const Row rows[]={ROWS};
static const Division divisions[]={DIVISIONS};
int main(void){
 for(size_t i=0;i<sizeof rows/sizeof rows[0];++i){Row r=rows[i];int value=917;
  if(fi_binary((FiRange){INT_MIN,INT_MAX},(FiOperation)r.op,r.a,r.b,&value)!=(r.ok?FI_OK:FI_RANGE)||value!=r.value){fprintf(stderr,"FI_NATIVE_FAIL arithmetic %zu\\n",i);return 1;}}
 for(size_t i=0;i<sizeof divisions/sizeof divisions[0];++i){Division r=divisions[i];int q=917,rem=919;
  if(fi_divide((FiRange){INT_MIN,INT_MAX},(FiDivision)r.policy,r.a,r.b,&q,&rem)!=(r.ok?FI_OK:FI_RANGE)||q!=r.q||rem!=r.r){fprintf(stderr,"FI_NATIVE_FAIL division %zu\\n",i);return 1;}}
 puts("FI_NATIVE_PASS arithmetic=972 division=972 independent_bigints");return 0;
}
'''.replace('ROWS',',\n'.join(rows)).replace('DIVISIONS',',\n'.join(division))
    c.write('native-oracle.json',json.dumps(dict(values=values,arithmetic=972,division=972,oracle='Python arbitrary precision arithmetic and floor division; C execution only after exact admission'),indent=2)+'\n')
    return c.write('native-oracle.c',source)

def run(c,root):
    lab=root/'labs/integer_kernel';ref=root/'assessor/integer_kernel'
    flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab)]
    for cc in ['gcc','clang']:
      c.group=cc+':observations';exe=c.root/(cc+'-observations')
      c.run([cc,*flags,ref/'integer.c',ref/'observations.c','-o',exe])
      text=c.run([exe],contains='FI_OBSERVE_PASS actual_limits promotions bitfields wider_signed local_pointer')
      c.require('FI_SIZE_BOUNDARIES executed=0 unavailable_source_values=2' in text,'native intmax cannot represent SIZE_MAX; no fabricated boundary execution')
      c.require(sum(line.startswith('TYPE ') for line in text.splitlines())==11,'all fundamental integer types observed')
      c.group=cc+':analysis-only';c.run([cc,*flags,'-c',ref/'analysis_only.c','-o',c.root/(cc+'-analysis.o')])
      for edition in ['c17','c23']:
        for early in [False,True]:
          c.group=cc+':'+edition+':'+str(early);exe=c.root/(cc+'-'+edition+'-'+str(early))
          c.run([cc,'-std='+edition,'-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(lab),'-I'+str(root/'labs/foundation_depth/include'),*(['-DFI_C23'] if edition=='c23' else []),*(['-DFI_BUILTIN_EARLY'] if early else []),ref/'integer.c',ref/'comparisons.c',root/'assessor/foundation_depth/depth.c','-o',exe])
          c.run([exe],expected=int(early),contains='manual==extension' if early else 'FI_COMPARE_PASS operations=196608 extents=6561')
