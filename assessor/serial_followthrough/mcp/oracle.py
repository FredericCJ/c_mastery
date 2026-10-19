"""Separate integer counts/CRC recurrence oracle, not hardware evidence."""
import json,pathlib,sys
expected={'layout':[8*16,17*72+29*16,2048,44775],'init':[490,16,2048,17000000],'failure':[2*490,3,1026,0],'transfer':[sum(sum(n<=p for n in [0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64]) for p in [8,12,16,20,24,32,48,64]),128,1,2**32-1],'receive':[64,1,73,2],'bench':[12,2*sum([0,8,12,16,20,64]),768,773],'spi':[490,16,1,1]}
expected['atomic']=[36,24,1,999]
text=pathlib.Path(sys.argv[1]).read_text();wanted=list(expected) if sys.argv[2]=='all' else [sys.argv[2]]
rows={}
for line in text.splitlines():
 if line.startswith('OBS mcp_'):
  parts=line.split();key=parts[1][4:];assert key not in rows;rows[key]=list(map(int,parts[2:]))
assert set(rows)==set(wanted),(rows,wanted)
for k in wanted:assert rows[k]==expected[k],(k,rows[k],expected[k])
print(json.dumps(dict(status='pass',packets=len(wanted),keys=wanted)))
