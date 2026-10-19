"""Exact rational oracle, independent of the C division and register packing."""
import fractions,json,pathlib,sys
F=fractions.Fraction
rows=[s.split()[1:] for s in pathlib.Path(sys.argv[1]).read_text().splitlines() if s.startswith('OBS ')]
baud=[];errors=[]
for clock in [42000000,21000000,16000000]:
 n=int(F(clock,115200)+F(1,2));errors.append(dict(clock=clock,baud=str(F(clock,n)),ppm=str((F(clock,n)/115200-1)*1000000)))
 for over in [0,1]:baud.append([clock,over,n,n if not over else (n//8)*16+n%8])
expected={'frames':[[2048,0x340c,365,19]],'baud':baud,'tx':[[n,n,n,(n+7)//8] for n in range(1,17)],'rx':[[33,0,255,8]],'deadline':[[8,8,9,7]],'mapped':[[365,0xffffff77,0x20000000,2]]}
keys=list(expected) if sys.argv[2]=='all' else [sys.argv[2]]
actual={k:[list(map(int,r[1:])) for r in rows if r[0]==k] for k in keys}
assert actual=={k:expected[k] for k in keys},actual
if 'baud' in keys:assert [list(map(int,r[1:])) for r in rows if r[0]=='baud_limit']==[[14,22,4,0]]
print(json.dumps(dict(status='pass',packets=sum(len(x) for x in actual.values()),keys=keys,rational_errors=errors if 'baud' in keys else [])))
