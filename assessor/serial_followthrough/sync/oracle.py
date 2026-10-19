"""Independent exact observations, not an elapsed-physical-time oracle."""
import json,pathlib,sys
rows=[s.split()[1:] for s in pathlib.Path(sys.argv[1]).read_text().splitlines() if s.startswith('OBS ')]
expected={'edges':[[2048,14,16,150]],'config':[[3840,8204,8,1]],'transfer':[[n]*4 for n in range(1,9)],'failures':[[4,3,1,0]],'mapped':[[365,3328,0xfffff777,0xffeaffff]]}
keys=list(expected) if sys.argv[2]=='all' else [sys.argv[2]]
actual={k:[list(map(int,r[1:])) for r in rows if r[0]==k] for k in keys}
assert actual=={k:expected[k] for k in keys},actual
print(json.dumps(dict(status='pass',packets=sum(len(x) for x in actual.values()),keys=keys)))
