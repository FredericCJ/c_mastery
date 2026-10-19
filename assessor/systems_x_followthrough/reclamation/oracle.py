import itertools,sys
from pathlib import Path
actual={}
for line in Path(sys.argv[1]).read_text().splitlines():
 if line.startswith('H '):
  mode,reader,mask,*result=map(int,line.split()[1:]);actual[mode,reader,mask]=tuple(result)
expected={}
for mode,reader in itertools.product(range(2),repeat=2):
 for positions in itertools.combinations(range(5),2):
  mask=sum(1<<i for i in positions);events=[];r=iter('OPR');w=iter('DC')
  for i in range(5):events.append(next(w) if i in positions else next(r))
  linked=True;retired=False;exists=True;held=False;observed=False;validated=False;a=b=c=-1;value=999;count=999
  for event in events:
   if event=='O':a=0 if linked else 9;observed=linked
   elif event=='P' and observed:b=0 if linked and exists else 8;validated=b==0;held=validated
   elif event=='R' and validated:
    assert exists;c=0;value=11
   elif event=='D':linked=False;retired=True
   elif event=='C':
    collect=retired and mode==0 and not held;count=int(collect)
    if collect:exists=False
  expected[mode,reader,mask]=(a,b,c,value,count)
assert actual==expected,(actual,expected)
print('LIST_ORACLE_PASS histories=40 exact_gate_serializations both_readers both_modes')
