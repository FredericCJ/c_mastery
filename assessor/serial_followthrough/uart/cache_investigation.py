"""Original one-line write-back model; neither an M7 emulator nor cache HAL."""
import json,sys
mutant=sys.argv[1] if len(sys.argv)>1 else ''
class Line:
 def __init__(self):self.ram=[0]*32;self.cache=[0]*32;self.valid=True;self.dirty=False
 def write(self,i,v):
  if not self.valid:self.cache=self.ram.copy();self.valid=True
  self.cache[i]=v;self.dirty=True
 def read(self,i):
  if not self.valid:self.cache=self.ram.copy();self.valid=True
  return self.cache[i]
 def clean(self):
  if self.valid and self.dirty and mutant!='CLEAN':self.ram=self.cache.copy();self.dirty=False
 def invalidate(self):
  if mutant!='INVALIDATE':self.valid=False;self.dirty=False
def require(ok,message):
 if not ok:print('CACHE_ASSERT:',message);raise SystemExit(43)
results=[]
for value in range(1,256):
 l=Line();l.write(0,value);require(l.ram[0]==0,'TX stale RAM counterexample')
 l.clean();require(l.ram[0]==value,'TX clean before DMA admission')
 l=Line();l.ram[0]=value;require(l.read(0)==0,'RX stale cached copy counterexample')
 l.invalidate();require(l.read(0)==value,'RX invalidate after acknowledged DMA completion')
 l=Line();l.write(31,0x5a);l.ram[0]=value;l.clean()
 require(l.ram[0]==0,'Shared dirty line clean overwrites DMA result')
 l=Line();l.write(31,0x5a);l.ram[0]=value;l.invalidate()
 require(l.read(0)==value and l.read(31)==0,'Shared-line invalidate loses adjacent dirty byte')
 results.append(value)
print('CACHE_INVESTIGATION_PASS '+json.dumps(dict(values=len(results),counterexamples_per_value=4,line_bytes=32,scope='Finite write-back model, no target maintenance instructions, barriers, routing or physical execution')))
