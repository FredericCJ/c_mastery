"""Independent integer/layout/state expectations; no production implementation import."""
import json,pathlib,re,sys
expected={'init':[6*16+6*2048,(1<<25)|(1<<21)|(1<<13)|2,0x8000+0x4000+0x800+12,1],
 'tx':[2+32,1+32,1,14+46],'rx':[4+1+16,64-4,1518-4,4+1+16],
 'recovery':[2,1,0,0],'drop':[6,1,1<<13,0],'bounds':[2,2,2**32-1,0],
 'failures':[1+13*2,1+13*2,2,1],'port':[3+1+3+2,720,1+2048,0],
 'chain':[len(range(4,100,4)),100-4,64-4,64],'publication':[2,3,1,0],
 'visibility':[7,9,5,7],'consumer':[1,0,1,1],'order':[0,(1<<1)|(1<<5)|(1<<6),7,8]}
text=pathlib.Path(sys.argv[1]).read_text();case=sys.argv[2];rows={}
for line in text.splitlines():
 if line.startswith('OBS ring_'):
  f=line.split();key=f[1][5:];assert key not in rows,line;rows[key]=list(map(int,f[2:]))
keys=list(expected) if case=='all' else [case]
assert set(rows)==set(keys),(rows,keys)
for k in keys:assert rows[k]==expected[k],(k,rows[k],expected[k])
print(json.dumps(dict(status='pass',packets=len(rows),keys=keys,limits='Finite original model/target packets, not physical observations or independent learner assessment')))
