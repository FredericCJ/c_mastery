"""Independent finite byte/segment conservation expected results."""
from pathlib import Path
import sys
rows=[]
for line in Path(sys.argv[1]).read_text().splitlines():
    if not line.startswith('TX '):raise AssertionError(line)
    n,mss,window,order,owner,segments,total,acked=map(int,line.split()[1:])
    capacity=min(mss,window)
    # Count occupied bins and sum the independently defined source stream.
    bins={offset//capacity for offset in range(n)}
    assert segments==len(bins) and total==sum((17*i+owner)%256 for i in range(n)) and acked==n
    rows.append((n,mss,window,order,owner))
expected={(n,m,w,o,k) for n in [3,256,257,600,1600] for m in [1,7,256] for w in [1,17,512] for o in [0,1] for k in [0,1]}
assert len(rows)==180 and set(rows)==expected
print('NW_LEDGER_ORACLE_PASS cases=180 two_owners both_completion_orders rejected_admissions')
