"""Compare actual instruments under equal admission and event schedules."""
from pathlib import Path
import json,sys
items=[json.loads(Path(p).read_text()) for p in sys.argv[1:]]
assert len(items)==8;rows=[]
for i in range(0,8,2):
 a,b=items[i:i+2];assert a['instrument']==0 and b['instrument']==1 and a['passed'] and b['passed']
 for x,y in zip(a['workloads'],b['workloads']):
  for key in ['workload','accepted','dropped','declared_events','declared_window_ticks']:assert x[key]==y[key]
  rows.append(dict(compiler_profile_pair=i//2,workload=x['workload'],without_publication_brackets=x['handler_ticks'],with_publication_brackets=y['handler_ticks'],overhead_bracket=b['overhead_ticks']))
print(json.dumps(dict(passed=True,pairs=4,rows=rows,limits='Same queue/admission and event-window contract. Both retain entry/exit instrumentation. Difference is an observed virtual-tick distribution, not exact instruction cost or uninstrumented physical behavior.'),indent=2))
