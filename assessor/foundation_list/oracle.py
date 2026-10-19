"""Independent whole-token regex/bignum oracle, never a target dependency."""
import itertools,json,re
def token(t):
 if not re.fullmatch(r'[+-]?[0-9]+',t):return 2,None
 v=int(t)
 return (3,None) if not -32768<=v<=32767 else (0,v)
def model(tokens,collect=False,capacity=4):
 errors=[];values=[]
 for i,t in enumerate(tokens):
  s,v=token(t)
  if not s and i>=capacity:s=4
  if s:
   errors.append((i,s))
   if not collect:break
  else:values.append(v)
 if errors:return errors[0][1],91,92,errors
 # Direct sums of slices, not the C digit accumulator or checked-add recurrence.
 if any(not -32768<=sum(values[:k])<=32767 for k in range(1,len(values)+1)):
  return 5,91,92,[(len(values),5)]
 return (0 if values else 1),len(values),sum(values),[]
def generate(c):
 alphabet=['','+','-','0','-0','+0001','32767','-32768','32768','1x',' 1']
 cases=[list(x) for n in range(3) for x in itertools.product(alphabet,repeat=n)]
 cases += [list(map(str,x)) for x in itertools.product([-32768,-1,0,1,32767],repeat=3)]
 cases += [['0']*4,['0']*5,['x','99999','+','1','2','--3','0','0','0','0','0'],['9999999999999999999x']]
 lines=['#include "list.h"','#include <stdio.h>','#define C(x) do { if(!(x)){fprintf(stderr,"FT_ORACLE:%u:%s\\n",__LINE__,#x);return 1;} } while(0)','int main(void) { FtResult r; FtErrors e; FtStatus s;']
 for i,ts in enumerate(cases):
  literals=','.join(json.dumps(t) for t in ts) or '0';lines.append('{ const char *a[] = {'+literals+'};')
  for collect in [0,1]:
   s,n,total,errors=model(ts,bool(collect));lines += ['r=(FtResult){91,92};',f's=ft_tokens(a,{len(ts)},{collect},&r,&e);',f'C(s=={s} && r.count=={n} && r.sum=={total} && e.total=={len(errors)} && e.stored=={min(8,len(errors))});']
   for j,(index,code) in enumerate(errors[:8]):lines.append(f'C(e.items[{j}].token=={index} && e.items[{j}].status=={code});')
  lines.append('}')
 lines += [f'puts("FT_ORACLE_PASS cases={len(cases)} modes=2");return 0;','}']
 source=c.write('independent-oracle.c','\n'.join(lines)+'\n')
 c.write('independent-oracle.json',json.dumps(dict(cases=cases,expected=[dict(fail_fast=model(t),collect_all=model(t,True)) for t in cases],method='Whole-token regex and Python integers; direct mathematical slice sums; no C recurrence reuse'),indent=2)+'\n')
 return source,len(cases)
