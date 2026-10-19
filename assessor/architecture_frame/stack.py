"""Extend the frozen producer's closed normal-call graph for the new preflight."""
import argparse,json,pathlib,re,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('directory',type=pathlib.Path);p.add_argument('fp',type=int);a=p.parse_args();d=a.directory;usage={};graph={};current=None
 for f in d.glob('*.su'):
  for row in f.read_text().splitlines():
   name,size,kind=row.split('\t');assert kind=='static',(name,kind);usage[name.rsplit(':',1)[-1]]=int(size)
 usage.update(xf_reset=0,xf_trigger=16,xf_irq_a=8,xf_irq_b=8,xf_irq_c=8)
 assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',d/'image.elf'],text=True)
 for line in assembly.splitlines():
  m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
  if m:current=m[1];graph[current]=set();continue
  m=re.search(r'\s(?:bl|blx|b(?:\.[nw])?)\s+[0-9a-f]+ <([^>]+)>',line)
  if m and current:
   callee=m[1].split('+')[0]
   if callee!=current:graph[current].add(callee)
  assert not re.search(r'\sblx?\s+r\d+',line),'unaccounted indirect call'
 def cost(name,visiting=()):
  assert name in usage and name not in visiting,('unbounded/unaccounted frame',name)
  return usage[name]+max([cost(c,visiting+(name,)) for c in graph.get(name,set())] or [0])
 basic=cost('xf_target_main');nested=usage['xf_target_main']+usage['mf_original_target_main']+16+8+usage['xf_capture']+(104 if a.fp else 32)+8+cost('xf_capture');bound=max(basic,nested);assert bound<8192
 out=dict(passed=True,local_bytes=usage,direct_calls={k:sorted(v) for k,v in graph.items()},preflight_and_sequential_max=basic,nested_msp_bound=nested,msp_bound=bound,psp_hardware_max=108,limits=['Closed direct-call experiment and normal paths only; preflight occurs before IRQ setup.','Static bound is conservative, not a measured high-water or physical latency.']);(d/'stack.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out))
if __name__=='__main__':main()
