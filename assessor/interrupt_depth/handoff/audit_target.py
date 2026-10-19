"""Inspect actual objects/imports/instructions and retain bounded stack metadata."""
from pathlib import Path
import subprocess,sys,re,json
d=Path(sys.argv[1]);elf=d/'image.elf'
def output(*cmd):return subprocess.check_output(list(map(str,cmd)),text=True)
assert not output('arm-none-eabi-nm','-u',elf).strip()
headers=output('arm-none-eabi-readelf','-l',elf);assert 'INTERP' not in headers and 'DYNAMIC' not in headers
assembly=output('arm-none-eabi-objdump','-drwC',elf)
for symbol in ['xi_timer','xi_primask_enter','xi_primask_leave','xi_basepri_enter','xi_basepri_leave','xi_wide_add']:
 assert '<'+symbol+'>:' in assembly,symbol
assert 'BASEPRI_MAX' in assembly or 'basepri_max' in assembly
assert 'dmb' in assembly and 'dsb' in assembly and 'isb' in assembly
assert not re.search(r'__atomic_|__sync_|__aeabi_[a-z]*div',assembly)
imports={};stacks={}
for p in d.glob('*.o'):
 imports[p.name]=output('arm-none-eabi-nm','-u',p).splitlines()
for p in d.glob('*.su'):
 for line in p.read_text().splitlines():
  parts=line.split('\t');assert len(parts)==3
  name=parts[0].rsplit(':',1)[-1];size=int(parts[1]);assert parts[2] in ['static','dynamic,bounded'] and size<8192
  stacks[name]=dict(local_bytes=size,kind=parts[2])
assert 'xi_timer' in stacks and 'xi_capture_begin' in stacks and 'xi_commit' in stacks
graph={};backedges=[];indirect=[];current=None
for line in assembly.splitlines():
 m=re.match(r'^([0-9a-f]+) <([^>]+)>:',line)
 if m:current=m[2];graph[current]=set();continue
 if not current:continue
 m=re.search(r'^\s*([0-9a-f]+):.*\t(b(?:l|lx|\.w|\.n|eq\.n|ne\.n|cc\.n|cs\.n|hi\.n|ls\.n))\s+([0-9a-f]+) <([^>]+)>',line)
 if m:
  destination=m[4].split('+')[0]
  if destination!=current:graph[current].add(destination)
  elif int(m[3],16)<int(m[1],16):backedges.append(dict(function=current,instruction=line.strip()))
 if re.search(r'\tblx\s+r\d+',line):indirect.append(line.strip())
assert not indirect,'unresolved indirect call'
def closure(name,active=()):
 assert name in graph and name not in active,('unknown/recursive',name)
 result={name}
 for callee in graph[name]:result|=closure(callee,(*active,name))
 return result
timer=closure('xi_timer');nested=closure('xi_high')
def local(name):
 candidates=[r['local_bytes'] for n,r in stacks.items() if n==name or name.startswith(n+'.')]
 assert candidates,('missing stack metadata',name)
 return max(candidates)
# Sum each reachable local once conservatively exceeds any nonrecursive path.
# Add the separately admitted higher-priority handler and two basic32-byte
# hardware frames plus worst4-byte alignment each. Foreground usage is separate.
bound=sum(local(n) for n in timer)+sum(local(n) for n in nested)+2*36
assert bound<8192
print(json.dumps(dict(passed=True,object_imports=imports,stack_entries=stacks,complete_call_graph={n:sorted(graph[n]) for n in sorted(timer|nested)},backedges=[r for r in backedges if r['function'] in timer|nested],handler_increment_upper_bytes=bound,limits='Conservative sum of all reachable locals, including fault branches; two declared basic hardware frames/alignment. Foreground depth, FP, NMI/fault nesting and physical WCET excluded. Normal-path loop bounds are eight-byte copies and fixed emitted28-byte helper arguments; explicit fatal assertion trap is not normal service.'),indent=2))
