"""Independent finite tables for T71; optional actual shared-library comparison."""
import argparse,ctypes as c,itertools,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class Builder(c.Structure):_fields_=[(x,c.c_uint) for x in ('mask','address','length','timeout')]
class Config(c.Structure):_fields_=[(x,c.c_uint) for x in ('address','length','timeout')]
class Session(c.Structure):_fields_=[(x,c.c_uint) for x in ('role','phase','remaining')]
class Machine(c.Structure):_fields_=[(x,c.c_uint) for x in ('active','dispatching','pings')]
class Thread(c.Structure):_fields_=[('pc',c.c_uint),('count',c.c_uint),('sum',c.c_uint32)]
def main():
 p=argparse.ArgumentParser();p.add_argument('--library');args=p.parse_args();lib=c.CDLL(args.library) if args.library else None
 counts={k:0 for k in ('builder','session','hsm','thread')}
 if lib:
  lib.pt_builder_finish.argtypes=[c.POINTER(Builder),c.POINTER(Config)]
  lib.pt_session_step.argtypes=[c.POINTER(Session),c.c_uint,c.c_uint]
  lib.pt_dispatch.argtypes=[c.POINTER(Machine),c.c_uint,c.POINTER(c.c_ubyte),c.c_size_t,c.POINTER(c.c_size_t)]
  lib.pt_thread_sum.argtypes=[c.POINTER(Thread),c.c_uint,c.c_uint32,c.POINTER(c.c_uint32)]
 for address,length,mask in itertools.product(range(0,256,4),range(1,65),range(8)):
  expected=3 if mask!=7 else 2 if address+length>256 else 0
  if lib:
   b=Builder(mask,address,length,10);out=Config(91,92,93);actual=lib.pt_builder_finish(c.byref(b),c.byref(out));assert actual==expected
   assert (out.address,out.length,out.timeout)==((address,length,10) if not expected else (91,92,93))
  counts['builder']+=1
 tags=[1,2,3,4,5]
 for role,phase,remaining,sending,tag in itertools.product(range(2),range(6),range(9),range(2),range(1,6)):
  expected=3;after=(phase,remaining)
  if phase in (2,3) and remaining==0:expected=1
  elif phase<5 and tag==tags[phase] and sending==int((phase in (0,2,4)) != bool(role)):
   expected=0;after=(phase+1,remaining)
   if phase==1:after=(2 if remaining else 4,remaining)
   if phase==3:after=(2 if remaining>1 else 4,remaining-1)
  if lib:
   s=Session(role,phase,remaining);assert lib.pt_session_step(c.byref(s),sending,tag)==expected
   assert (s.phase,s.remaining)==after
  counts['session']+=1
 # Independent explicit transition traces, not an LCA algorithm copied from C.
 table={(1,0):(3,[129,2,3]),(3,1):(4,[131,4]),(4,1):(4,[132,4]),(4,2):(3,[132,3]),(3,3):(5,[131,130,5]),(4,3):(5,[132,130,5]),(2,3):(5,[130,5]),(5,4):(1,[133,1])}
 for state,event,cap in itertools.product(range(1,6),range(6),range(7)):
  target,trace=table.get((state,event),(state,[]));expected=0 if (state,event) in table or event==5 else 3
  if len(trace)>cap:expected=2
  if lib:
   m=Machine(state,0,0);buf=(c.c_ubyte*6)(*([99]*6));used=c.c_size_t(91)
   assert lib.pt_dispatch(c.byref(m),event,buf,cap,c.byref(used))==expected
   assert m.active==(target if not expected else state)
   assert used.value==(len(trace) if not expected else 91)
   assert list(buf)==(trace+[99]*(6-len(trace)) if not expected else [99]*6)
  counts['hsm']+=1
 for values in itertools.product([0,1,17,2**32-1],repeat=3):
  total=0
  if lib:frame=Thread();out=c.c_uint32(91)
  for index,value in enumerate(values):
   if total+value>2**32-1:
    if lib:assert lib.pt_thread_sum(c.byref(frame),1,value,c.byref(out))==2 and frame.sum==total and frame.count==index and out.value==91
    break
   total+=value
   if lib:
    assert lib.pt_thread_sum(c.byref(frame),0,99,c.byref(out))==4
    assert lib.pt_thread_sum(c.byref(frame),1,value,c.byref(out))==(7 if index==2 else 4)
    assert out.value==(total if index==2 else 91)
  counts['thread']+=1
 for i in range(1,9):assert json.loads((ROOT/f'fixtures/protocols/PR-T71-{i:02}.json').read_text(encoding='utf-8'))['exercise']==f'X-T71-{i:03}'
 print(json.dumps(dict(passed=True,executed_against_c=bool(lib),cases=counts,total=sum(counts.values()),limits='Finite independent models cover builder/session/HSM/thread; other mechanism anchors use separately derived C traces and callback counts, not an exhaustive oracle.')))
if __name__=='__main__':main()
