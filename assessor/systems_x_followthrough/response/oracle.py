import sys
from pathlib import Path
count=0
for line in Path(sys.argv[1]).read_text().splitlines():
 if not line.startswith('R '):continue
 own,cost,period,jitter,sw,rc,out=map(int,line.split()[1:]);base=own+2*sw;r=base
 while r<=100:
  candidate=base+((r+jitter+period-1)//period)*(cost+2*sw)
  if candidate==r:break
  assert candidate>r;r=candidate
 expected=(0,r) if r<=100 else (2,999)
 assert (rc,out)==expected,(line,expected)
 count+=1
assert count==1728,count
print('RESPONSE_ORACLE_PASS cases=1728 direct_integer_ceiling unchanged_refusal=1')
