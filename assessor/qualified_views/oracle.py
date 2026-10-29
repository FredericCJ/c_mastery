"""Snapshot-derived overlap oracle; expected values do not use copy direction."""
from itertools import product
import json
def generate(c):
 rows=[]
 for extent in range(9):
  original=list(range(10,18))
  for dst,src,count in product(range(11),repeat=3):
   good=dst<=extent and src<=extent and count<=extent-dst and count<=extent-src
   after=original.copy()
   if good:after[dst:dst+count]=original[src:src+count]
   rows.append(dict(extent=extent,destination=dst,source=src,count=count,status='FV_OK' if good else 'FV_RANGE',expected=after))
 c.write('move-oracle.json',json.dumps(rows,separators=(',',':'))+'\n')
 text=['#include "views.h"\n#include <stdio.h>\nstatic const struct {size_t e,d,s,n;enum fv_status status;int out[8];} rows[]={']
 for r in rows:text.append('{'+','.join(str(r[k]) for k in ['extent','destination','source','count'])+','+r['status']+',{'+','.join(map(str,r['expected']))+'}},')
 text.append('};\nint main(void){for(size_t i=0;i<sizeof rows/sizeof rows[0];++i){int a[8]={10,11,12,13,14,15,16,17};if(fv_move(a,rows[i].e,rows[i].d,rows[i].s,rows[i].n)!=rows[i].status)return 1;for(size_t j=0;j<8;++j)if(a[j]!=rows[i].out[j])return 2;}')
 text.append(f'puts("FV_ORACLE_PASS snapshot_ranges={len(rows)}");return 0;}}')
 return c.write('move-oracle.c','\n'.join(text)+'\n'),len(rows)
