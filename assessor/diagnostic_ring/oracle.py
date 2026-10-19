import json,pathlib,re,sys
expected={'tx':[60,1,3,2],'rx':[1,11,1,2]};keys=list(expected) if sys.argv[2]=='all' else [sys.argv[2]]
text=pathlib.Path(sys.argv[1]).read_text();seen={k:list(map(int,v.split())) for k,v in re.findall(r'DR_VALUES (\w+) ([0-9 ]+)',text)}
assert set(seen)==set(keys)
for k in keys:assert seen[k]==expected[k]
print(json.dumps(dict(status='pass',keys=keys,packets=len(seen),derivation='60=14+46 Ethernet minimum; eleven acquired leases include eight replay refusals, one malformed and one release-failed frame; malformed hardware frame is not a lease')))
