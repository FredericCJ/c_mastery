"""Conservative selected linked call paths with explicit callback binding."""
import re
CALLBACKS={
 'af_ring_door':('read_reg','write_reg','fence'),
 'af_mask_door':('read_reg','write_reg','fence'),
 'af_clear_door':('read_reg','write_reg','fence'),
 'pd_submit':('configure','enable','visibility'),
 'clean':('visibility',),
 'pd_poll':('snapshot','stop','configure','visibility'),
 'pd_cancel':('stop',),
 'pd_finalize':('visibility',),
 'fw_cache_range':('emit',),
}
def bounds(directory,frames):
 functions={};current=None
 for line in (directory/'disassembly.txt').read_text().splitlines():
  h=re.match(r'^([0-9a-f]+) <([^>]+)>:',line)
  if h:current=int(h[1],16);functions[current]={'name':h[2],'calls':set(),'indirect':[]};continue
  if current is None:continue
  m=re.search(r'\s(bl|blx|b(?:\.[nw])?|b(?:eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le)(?:\.[nw])?)\s+([0-9a-f]+) <([^>]+)>',line)
  if m and '+' not in m[3] and (int(m[2],16)!=current or m[1] in ('bl','blx')):functions[current]['calls'].add(int(m[2],16))
  z=re.search(r'\s(?:cbz|cbnz)\s+r\d+,\s*([0-9a-f]+) <([^>]+)>',line)
  if z and '+' not in z[2] and int(z[1],16)!=current:functions[current]['calls'].add(int(z[1],16))
  if re.search(r'\b(?:blx|bx)\s+r(?:[0-9]|1[0-3])\b',line):functions[current]['indirect'].append(line.strip())
 sizes={}
 for f in frames:sizes[f['function']]=max(sizes.get(f['function'],0),f['bytes'])
 bound_callbacks=[]
 for a,f in functions.items():
  if not f['indirect']:continue
  base=f['name'].split('.')[0]
  if base not in CALLBACKS:raise ValueError('Unbound indirect call '+f['name'])
  targets=[]
  for name in CALLBACKS[base]:
   matches=[p for p,t in functions.items() if t['name']==name or t['name'].startswith(name+'.')]
   if not matches:raise ValueError('Missing callback '+base+' -> '+name)
   f['calls'].update(matches);targets.extend(functions[p]['name'] for p in matches)
  bound_callbacks.append(dict(caller=f['name'],targets=targets,premise='Exact private provider manifest and original target callbacks; no arbitrary caller-supplied callback admitted in this image.'))
 def visit(address,active):
  f=functions[address];name=f['name'];base=name.split('.')[0]
  if address in active:raise ValueError('Recursive selected path '+name)
  if name not in sizes and base not in sizes:raise ValueError('No compiler frame for '+name)
  children=[visit(c,active|{address}) for c in f['calls']]
  child=max(children,key=lambda x:x['bytes']) if children else {'bytes':0,'chain':[]}
  own=sizes.get(name,sizes.get(base));return {'bytes':own+child['bytes'],'chain':[{'function':name,'bytes':own}]+child['chain']}
 roots={}
 for name in ('af_entry','primary','secondary','producer_task','sink_task','worker_task','device_task','idle_task','SysTick_Handler','af_hsem','af_fault','cmx_port_choose'):
  matches=[a for a,v in functions.items() if v['name']==name]
  if matches:roots[name]=max((visit(a,set()) for a in matches),key=lambda x:x['bytes'])
 task=[v['bytes'] for k,v in roots.items() if k.endswith('_task')]
 psp=208+max(task,default=0)
 # Conservative overlap: selected SysTick, HSEM, fault and switch C paths,
 # with four extended exception-frame allowances even though masks exclude
 # several combinations. Startup gets its own full direct-call bound.
 msp=max(roots.get('af_entry',{'bytes':0})['bytes'],432+sum(roots.get(k,{'bytes':0})['bytes'] for k in ('SysTick_Handler','af_hsem','af_fault','cmx_port_choose')))
 return dict(roots=roots,bound_callbacks=bound_callbacks,PSP_selected_bytes=psp if task else None,MSP_selected_bytes=msp,premises='Exact linked calls and static compiler frames, conservative tail-call overcount. Original manifest binds each callback; no arbitrary new callback, recursion, interrupt source or FPU use admitted. PSP includes208-byte extended-context allowance. MSP sums four108-byte exception allowances plus selected C chains; startup considered separately. Not a universal stack/WCET proof.')
