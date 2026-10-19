"""Check recorded IRQ acquisition intervals without inventing event instants."""
import json,sys
from pathlib import Path
assert 'XK_ARITHMETIC_PASS large_division_boundaries=4' in Path(sys.argv[1]).read_text()
rows=[]
for line in Path(sys.argv[1]).read_text().splitlines():
    if line.startswith('XK_CAPTURE '):rows.append([int(x,16) for x in line.split()[1:]])
assert len(rows)==7 and [r[0] for r in rows]==list(range(7))
for i,posted,entry,begin,sample,end,lower,upper in rows:
    assert 0<=posted<=entry<=begin<=sample<=end<1000000
    assert (lower,upper)==(begin*40,end*40)
assert 'XK_TARGET_PASS actual_irq=7 queued=4 dropped=3 odd_snapshot_refused=1' in Path(sys.argv[1]).read_text()
print(json.dumps(dict(passed=True,rows=rows,entry_to_sample_tick_intervals=[[r[3]-r[2],r[5]-r[2]] for r in rows],
 units='25MHz virtual timer tick;40ns per tick',
 limits=['Posted software arm time is not the external physical event instant.','Counter is an actual32-bit CMSDK timer in a bounded no-wrap run; constant high0 is not a64-bit hardware timer.','The odd CPF writer-state read is an actual interrupt, but the partial writer state is deliberately staged through its atomic fields.','Three drop-newest losses are detected by sequence gap; successful counter sampling alone does not reveal them.','No physical logic analyzer/board observation.']),indent=2))
