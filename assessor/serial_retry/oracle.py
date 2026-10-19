"""Independent finite packet arithmetic, not a proof of all schedules."""
import json,pathlib,re,sys
expected={'stretch':[256*127,5,10,128],'transaction':[4*3,3*9,1,6],'retry':[2,9+1,1,5]}
text=pathlib.Path(sys.argv[1]).read_text();keys=list(expected) if sys.argv[2]=='all' else [sys.argv[2]]
seen={k:list(map(int,v.split())) for k,v in re.findall(r'SR_VALUES (\w+) ([0-9 ]+)',text)}
assert set(seen)==set(keys)
for k in keys:assert seen[k]==expected[k],(k,seen[k])
print(json.dumps(dict(status='pass',keys=keys,packets=len(seen))))
