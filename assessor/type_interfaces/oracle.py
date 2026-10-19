"""Finite unit wrapper expectations from mathematical equality/product, host only."""
import json
def generate(c):
 distances=[0,1,2,3,7,12,13,999,1000];times=[0,1,2,3,7,999,1000];speeds=[0,1,3,7,500,999,1000]
 rows=[];lines=['#include "types.h"','#include <stdio.h>','#define C(x) do{if(!(x)){fprintf(stderr,"TD_ORACLE:%d\\n",__LINE__);return 1;}}while(0)','int main(void){TdSpeed s;TdMeters d;TdStatus r;']
 for distance in distances:
  for t in times:
   q=next((q for q in range(1001) if t>0 and q*t==distance),None)
   lines += ['s.value=91;',f'r=td_speed((TdMeters){{{distance}}},(TdSeconds){{{t}}},&s);',f'C(r=={"TD_OK" if q is not None else "TD_RANGE"} && s.value=={q if q is not None else 91});']
   rows.append(dict(op='speed',distance=distance,time=t,exact_integer_solution=q))
 for speed in speeds:
  for t in times:
   value=speed*t;ok=t>0 and value<=1000
   lines += ['d.value=92;',f'r=td_distance((TdSpeed){{{speed}}},(TdSeconds){{{t}}},&d);',f'C(r=={"TD_OK" if ok else "TD_RANGE"} && d.value=={value if ok else 92});']
   rows.append(dict(op='distance',speed=speed,time=t,value=value,admitted=ok))
 lines += [f'puts("TD_ORACLE_PASS equations={len(rows)}");return 0;','}'];c.write('unit-oracle.json',json.dumps(rows,indent=2)+'\n');return c.write('unit-oracle.c','\n'.join(lines)+'\n'),len(rows)
