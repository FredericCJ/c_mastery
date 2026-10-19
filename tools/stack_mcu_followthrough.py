"""Selected linked call graph with conservative tail-call and FP allowances."""
import re
def bounds(directory,frames):
 text=(directory/'disassembly.txt').read_text();functions={};current=None
 for line in text.splitlines():
  h=re.match(r'^([0-9a-f]+) <([^>]+)>:',line)
  if h:current=int(h[1],16);functions[current]={'name':h[2],'calls':set(),'indirect':[]};continue
  if current is None:continue
  # Branch-to-another-function is conservatively counted as simultaneous
  # even if the compiler restored the caller before a tail branch.
  m=re.search(r'\s(b(?:l|lx|\.w|eq(?:\.w)?|ne(?:\.w)?)?)\s+([0-9a-f]+) <([^>]+)>',line)
  if m and '+' not in m[3]:functions[current]['calls'].add(int(m[2],16))
  if re.search(r'\bblx\s+r\d+',line):functions[current]['indirect'].append(line.strip())
 sizes={}
 for x in frames:
  name=x['function'];sizes[name]=max(sizes.get(name,0),x['bytes'])
 # mf_async_probe has112-byte saved state plus64-byte temporary low-FP
 # observation area. Helpers' original assembly frames are explicit.
 sizes.update(mf_async_probe=176,__aeabi_uldivmod=24,__aeabi_uidivmod=16)
 def visit(address,active):
  f=functions[address];name=f['name'];base=name.split('.')[0]
  if address in active:raise ValueError('Unbounded recursion in selected chain: '+name)
  if f['indirect']:raise ValueError('Unresolved indirect selected call: '+name)
  if name not in sizes and base not in sizes:raise ValueError('No frame ledger for '+name)
  own=sizes.get(name,sizes.get(base));children=[visit(c,active|{address}) for c in f['calls']]
  child=max(children,key=lambda x:x['bytes']) if children else {'bytes':0,'chain':[]}
  return {'bytes':own+child['bytes'],'chain':[{'function':name,'frame_bytes':own}]+child['chain']}
 result={}
 for root in ('mf_tick','mf_nmi','mf_choose','mf_local_fault','first','second','supervisor'):
  matches=[a for a,v in functions.items() if v['name']==root]
  # Static helper name "first" also exists in cmx. Task entry is the first
  # linked occurrence; include both roots conservatively if names collide.
  result[root]=max((visit(a,set()) for a in matches),key=lambda x:x['bytes'])
 result['MSP_nested_conservative_bytes']=216+result['mf_tick']['bytes']+result['mf_nmi']['bytes']
 result['MSP_fault_or_switch_bytes']=108+max(result['mf_choose']['bytes'],result['mf_local_fault']['bytes'])
 result['PSP_conservative_bytes']=208+max(result[r]['bytes'] for r in ('first','second','supervisor'))
 result['premises']='Exact linked direct-call graph and static .su frames; tail calls overcount; no recursive or indirect selected edge admitted. Budget two108-byte MSP hardware allowances plus simultaneous SysTick/NMI C chains; PSP adds208-byte extended context allowance. Selected basic fault/switch branch separate. Newly added priorities/callbacks/interrupt sources invalidate the bound.'
 return result
