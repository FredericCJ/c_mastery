"""Independent functional last-receipt model; no store or C implementation import."""
from pathlib import Path
import itertools,json,sys
rows=[]
for initial in [0,2147483647,-2147483648]:
 for requests in itertools.product([(i,d) for i in [0,1,2] for d in [-1,1]],repeat=3):
  value=initial;last=None;rows.append('B '+str(initial))
  for ident,delta in requests:
   old=last[0] if last else 0
   ok=bool(ident) and ((last is not None and (ident,delta)==last[:2]) or ident==old+1 and -2147483648<=value+delta<=2147483647)
   result=91
   if ok:
    if last is not None and ident==last[0]:result=last[2]
    else:value+=delta;result=value;last=(ident,delta,result)
   rows.append('Q '+' '.join(map(str,[ident,delta,int(ok),result,value,last[0] if last else 0])))
Path(sys.argv[1]).write_text('\n'.join(rows)+'\n');print(json.dumps(dict(histories=648,cases=1944,oracle='integer mathematical value plus latest accepted tuple; no WAL or adapter algorithm')))
