"""Independent integer intervals, permutations and event relations; no C calls."""
import itertools,json,pathlib,sys
cases={
 'thresholds':[len(range(-25000,25001)),3600+600,600,2400],
 'connections':[3**3,sum(p==(1,2,3) for p in itertools.product(range(1,4),repeat=3)),3,115200],
 'capture232':[2**8*(1+8+1),16,(16+7)//8,5],
 'differential':[(7000*2+1)*2,10,200,7000],
 'exclusion':[200600+200+2000,2,2**32-1,0],
 'captures422':[5,0,sum(a!=b for a,b in zip([0,1,0,1],[1,0,1,1])),1]}
lines=pathlib.Path(sys.argv[1]).read_text().splitlines();packets={}
for line in lines:
 if line.startswith('OBS transceivers_'):
  fields=line.split();name=fields[1].removeprefix('transceivers_');assert name not in packets
  packets[name]=list(map(int,fields[2:]))
wanted=cases if sys.argv[2]=='all' else {sys.argv[2]:cases[sys.argv[2]]}
assert packets==wanted,(packets,wanted)
print(json.dumps(dict(status='pass',packets=len(packets),keys=list(packets))))
