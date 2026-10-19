"""Independent original-vector/layout and ownership certificate checks."""
import json,pathlib,sys
rows=[s.split()[1:] for s in pathlib.Path(sys.argv[1]).read_text().splitlines() if s.startswith('PACKET ')];case=sys.argv[2]
def values(name):return [list(map(int,s[1:])) for s in rows if s[0]==name]
if case=='reset':
 assert values('reset')==[[0,0x08000000,0,0x08000000,0x08000100,0x08000054]]
 assert values('remap')==[[b,v,0,base,base+0x100,v+84] for b,base in [(0,0x08000000),(1,0x20000000)] for v in [0,base]]
elif case=='capture':
 assert values('fixture')==[json.loads((pathlib.Path(__file__).resolve().parents[2]/'fixtures/algorithms/PR-T37-07.json').read_text())['input']['basic_words']+[0x20001000]]
 expected=[]
 for extended in [0,1]:
  for pad in [0,1]:
   for material in range(extended+1):expected.append([extended,pad,material,0x08000200,0x20001000+4*pad,26 if extended else 8,0x3f800000 if material else 0])
 assert values('extended')==expected
elif case=='ownership':
 hw=sum(1<<r for r in [0,1,2,3,12,14,15,16]);callee=sum(1<<r for r in [4,5,6,7,8,10,11,13]);scratch=sum(1<<r for r in [0,1,2,3,12,14,16]);assert values('ownership')==[[i,hw,callee+i*(1<<9),scratch,1-i] for i in [0,1]]
elif case=='nested':assert values('nested')==[[0,0x08000300,0x20003000,1,0x08000200,0x20001000,0xfffffffd,0x08000111]]
else:raise AssertionError(case)
print(json.dumps(dict(status='pass',packets=len(rows))))
