"""Conservative complete normal-path stack bound for this closed direct-call image."""
from pathlib import Path
import argparse,json,re,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('directory',type=Path);p.add_argument('--fp',type=int,required=True);a=p.parse_args();d=a.directory
 assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',str(d/'image.elf')],text=True)
 usage={}
 for f in d.glob('*.su'):
  for row in f.read_text().splitlines():
   name,size,kind=row.split('\t');assert kind=='static',(name,kind);usage[name.rsplit(':',1)[-1]]=int(size)
 usage.update(xf_reset=0,xf_trigger=16,xf_irq_a=8,xf_irq_b=8,xf_irq_c=8)
 graph={};current=None
 for line in assembly.splitlines():
  m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
  if m:current=m[1];graph[current]=set();continue
  # Branches within a function do not introduce another live activation.
  m=re.search(r'\s(?:bl|blx|b(?:\.[nw])?)\s+[0-9a-f]+ <([^>]+)>',line)
  if m and current:
   callee=m[1].split('+')[0]
   if callee!=current:graph[current].add(callee)
  assert not re.search(r'\sblx?\s+r\d+',line),'unaccounted indirect call'
 def cost(name,visiting=()):
  assert name in usage,('missing static frame',name)
  assert name not in visiting,('unbounded recursion',name)
  return usage[name]+max([cost(c,visiting+(name,)) for c in graph.get(name,set())] or [0])
 # A can preempt only inside xf_trigger; B is pended while A's xf_capture
 # activation is live. Its callees have returned at that point. Use the entire
 # static caller frame anyway, conservatively including outgoing argument space.
 frame_a=104+4;frame_b=104 if a.fp else 32
 handler=8+cost('xf_capture')
 nested=usage['xf_target_main']+16+8+usage['xf_capture']+frame_b+handler
 non_nested=usage['xf_target_main']+16+handler
 normal=max(cost('xf_target_main'),nested,non_nested)
 assert normal<8192 and frame_a<512*4
 out=dict(passed=True,profile_fp_callee=bool(a.fp),static_local_bytes=usage,direct_calls={k:sorted(v) for k,v in graph.items()},psp_hardware_max=frame_a,msp_conservative_max=normal,nested_msp_bound=nested,formula='main static + trigger16 + wrapperA8 + captureA static + B hardware + wrapperB8 + captureB closed call-tree maximum',limits=['Normal explicit two-level experiment only; unexpected faults terminate rather than recover.','Per-function static maximum may include non-simultaneous outgoing-argument space; this is a safe upper bound, not observed high-water or tight minimum.','No physical timing, arbitrary callbacks, recursion, DMA stack writes, or future interrupt additions covered.'])
 (d/'stack_ledger.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
if __name__=='__main__':main()
