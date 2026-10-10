"""Mathematical/abstract references for original fixtures, never learner C solutions."""
import itertools,json,math,re,zlib
def oracle(kind,x):
 if kind=='sort_effects':
  sizes=[2*n*x['pointer_bytes']+size if size>x['indirect_threshold'] else n*size for n,size in x['cases']]
  values=x['values'];rejected=x['over_capacity_values'];assert len(rejected)>x['replacement_limit']
  return {'temporary_bytes':sizes,'selection':['stack' if n<=x['glibc_stack_bytes'] else 'acquire_or_heap_fallback' for n in sizes],'sorted_values':sorted(values),'multiplicities':[[v,values.count(v)] for v in sorted(set(values))],'replacement_accepts':len(values)<=x['replacement_limit'],'over_capacity_rejected_unchanged':rejected[:]}
 if kind=='kernel_lifetime':
  # Declared finite lifetime predicates only: not a kernel/API simulator or unload proof.
  assert x['reference_edges']==[['producer','module_text'],['ordinary_work','module_text'],['ordinary_work','device_object']]
  state=dict(x['initial']);states=[];attempts=[];rejected=[]
  for i,event in enumerate(x['events']):
   busy=state['pending_work']>0 or state['running_work']>0
   if event=='smp_mb':pass
   elif event=='stop_and_join_producers':state['producers_live']=False
   elif event=='cancel_work_sync_return':
    if state['producers_live'] or not x['caller_sleepable'] or x['holds_worker_lock']:rejected.append(i)
    else:state['pending_work']=state['running_work']=0
   elif event=='release_object':
    if state['producers_live'] or busy:rejected.append(i)
    else:state['object_live']=False
   elif event=='attempt_unload':attempts.append(not state['producers_live'] and not busy and not state['object_live'])
   else:raise ValueError('Unknown lifetime event '+event)
   states.append(dict(state))
  return {'unload_attempts':attempts,'rejected_events':rejected,'states':states}
 if kind=='tree_snapshots':
  # Reconstruct invariants from declared snapshots; never implement insertion/rotation/deletion.
  keys=set();result=[];previous=None
  for s in x['snapshots']:
   op,key=s['operation'];duplicate=op=='insert' and key in keys
   if op=='insert':keys.add(key);status='duplicate_rejected' if duplicate else 'inserted'
   else:status='deleted' if key in keys else 'absent';keys.discard(key)
   nodes={n[0]:n[1:] for n in s['nodes']};visited=set();balances=[];valid=True
   def visit(k,lo=None,hi=None):
    nonlocal valid
    if k is None:return [],0
    if k in visited or k not in nodes:raise ValueError('cycle/shared/missing tree node')
    visited.add(k)
    if (lo is not None and k<=lo) or (hi is not None and k>=hi):valid=False
    l,r,stored=nodes[k];left,lh=visit(l,lo,k);right,rh=visit(r,k,hi);height=1+max(lh,rh)
    balances.append([k,lh-rh]);valid &= stored==height and abs(lh-rh)<=1
    return left+[k]+right,height
   inorder,height=visit(s['root']);valid &= visited==set(nodes) and inorder==sorted(keys)
   current=(s['root'],nodes)
   if duplicate:valid &= current==previous
   result.append({'status':status,'root':s['root'],'inorder':inorder,'height':height,'balances':sorted(balances),'valid':bool(valid)})
   previous=current
  return {'snapshots':result}
 if kind=='initial_stack':
  assert x['argc']==len(x['argv']) and x['auxv'][-1]==[0,0]
  words=[x['argc']]+x['argv']+[0]+x['envp']+[0]+[v for pair in x['auxv'] for v in pair]
  size=x['word_bytes'];return {'words':words,'argv_offset':size,'envp_offset':size*(len(x['argv'])+2),'auxv_offset':size*(len(x['argv'])+len(x['envp'])+3),'byte_extent':size*len(words),'stack_alignment_mod16':x['stack_base']%16}
 if kind=='search_policy':
  result=[]
  for kind,name in x['requests']:
   collection='host_programs' if kind=='program' else 'target_headers' if kind=='header' else 'target_libraries'
   prefix='host:' if kind=='program' else 'target:'
   result.append(prefix+name if name in x[collection] else None)
  return {'resolved':result,'unresolved':[i for i,v in enumerate(result) if v is None]}
 if kind=='plugin_contract':
  values={}
  for thread in x['tls_events']:values[thread]=values.get(thread,x['tls_initial'])+1
  return {'version_available':[version in x['exports'].get(name,[]) for name,version in x['requirements']],'thread_values':values}
 if kind=='handle_snapshots':
  return {'validity':[[0<=slot<len(s['live']) and s['live'][slot] and s['generation'][slot]==gen for slot,gen in s['queries']] for s in x['snapshots']]}
 if kind=='persistence_model':
  # Abstract successful-step contract only, not a filesystem simulator or crash guarantee.
  assert x['stages']==['write_temp','sync_temp','rename','sync_directory']
  return {'states':[{'completed':n,'visible':'new' if n>=3 else 'old','guaranteed_durable':'new' if n>=4 else 'unspecified' if n>=3 else 'old'} for n in x['stops']]}
 if kind=='sum_budget':return {'total':sum(x['parts']),'remaining':x['limit']-sum(x['parts']),'fits':sum(x['parts'])<=x['limit']}
 if kind=='closure':
  seen=set(x['roots']);pending=list(seen)
  while pending:
   for n in x['edges'].get(pending.pop(),[]):
    if n not in seen:seen.add(n);pending.append(n)
  return {'reachable':sorted(seen),'forbidden':sorted(seen.intersection(x.get('forbidden',[])))}
 if kind=='layout':
  offsets=[];cursor=0
  for size,align in x['fields']:cursor+=(-cursor)%align;offsets.append(cursor);cursor+=size
  a=max(v[1] for v in x['fields']);return {'offsets':offsets,'alignment':a,'size':cursor+(-cursor)%a}
 if kind=='reloc':
  v=x['S']+x['A']-x['P'];lo=-(1<<(x['bits']-1));hi=(1<<(x['bits']-1))-1
  return {'value':v,'fits':lo<=v<=hi}
 if kind=='copy':
  y=x['bytes'][:];y[x['dst']:x['dst']+x['length']]=x['bytes'][x['src']:x['src']+x['length']]
  return {'bytes':y}
 if kind=='sorted':return {'values':sorted(x['values']),'multiset':[[v,x['values'].count(v)] for v in sorted(set(x['values']))]}
 if kind=='bounds':
  a=x['values'];k=x['key'];return {'lower':sum(v<k for v in a),'upper':sum(v<=k for v in a)}
 if kind=='matches':
  a=x['bytes'];b=x['pattern'];return {'offsets':[i for i in range(len(a)-len(b)+1) if a[i:i+len(b)]==b]}
 if kind=='set':return {'union':sorted(set(x['a'])|set(x['b'])),'intersection':sorted(set(x['a'])&set(x['b'])),'difference':sorted(set(x['a'])-set(x['b']))}
 if kind=='graph':
  edges=x['edges'];n=x['vertices'];inf=None;ds=[inf]*n;ds[x['start']]=0
  for _ in range(n-1):
   for a,b,w in edges:
    if ds[a] is not None and (ds[b] is None or ds[b]>ds[a]+w):ds[b]=ds[a]+w
  return {'distances':ds,'reachable':[i for i,d in enumerate(ds) if d is not None]}
 if kind=='partition':
  groups=[{i} for i in range(x['n'])]
  for a,b in x['unions']:
   u=next(s for s in groups if a in s);v=next(s for s in groups if b in s)
   if u is not v:groups.remove(u);groups.remove(v);groups.append(u|v)
  return {'classes':sorted([sorted(s) for s in groups])}
 if kind=='rank':return {'value':sorted(x['values'])[x['rank']] if 0<=x['rank']<len(x['values']) else None}
 if kind=='knapsack':
  options=[(sum(x['items'][i][0] for i in range(len(x['items'])) if mask>>i&1),sum(x['items'][i][1] for i in range(len(x['items'])) if mask>>i&1)) for mask in range(1<<len(x['items']))]
  return {'best':max(v for w,v in options if w<=x['capacity'])}
 if kind=='integer':
  s=x['text'];base=x['base'];v=int(s,base);return {'value':v,'fits':x['minimum']<=v<=x['maximum'],'canonical':str(v)}
 if kind=='crc8':
  crc=0
  for b in x['bytes']:
   crc^=b
   for _ in range(8):crc=((crc<<1)^7)&255 if crc&128 else (crc<<1)&255
  return {'crc':crc}
 if kind=='frames':
  data=[b for c in x['chunks'] for b in c];out=[];i=0
  while i<len(data):
   size=data[i];i+=1
   if size>x['max_frame']:return {'frames':out,'status':'oversize','remaining':len(data)-i}
   if len(data)-i<size:return {'frames':out,'status':'incomplete','remaining':len(data)-i+1}
   out.append(data[i:i+size]);i+=size
  return {'frames':out,'status':'complete','remaining':0}
 if kind=='arena':
  p=0;starts=[];high=0
  for size,align in x['requests']:
   start=p+(-p)%align
   if start+size>x['capacity']:starts.append(None)
   else:starts.append(start);p=start+size;high=max(high,p)
  return {'offsets':starts,'used':p,'high_water':high}
 if kind=='extents':
  es=sorted(x['free']);merged=[]
  for start,size in es:
   if merged and merged[-1][0]+merged[-1][1]==start:merged[-1][1]+=size
   else:merged.append([start,size])
  return {'coalesced':merged,'total_free':sum(v[1] for v in es),'largest_free':max((v[1] for v in merged),default=0)}
 if kind=='classes':
  usable=[next((c for c in x['classes'] if c>=v),None) for v in x['requests']]
  return {'assigned':usable,'payload':sum(x['requests']),'charged':sum(v for v in usable if v is not None),'waste':sum(c-r for c,r in zip(usable,x['requests']) if c is not None)}
 if kind=='transfer':
  progress=0;retries=0;status='complete'
  for action in x['results']:
   if isinstance(action,int):
    if action==0:status='zero_progress';break
    progress+=min(action,x['requested']-progress)
    if progress==x['requested']:break
   elif action=='EINTR':retries+=1
   else:status=action;break
  if progress<x['requested'] and status=='complete':status='incomplete'
  return {'transferred':progress,'retries':retries,'status':status}
 if kind=='umask':return {'mode':x['requested']&~x['mask']&511}
 if kind=='offset':
  offset=x['initial'];positions=[]
  for op,count in x['operations']:
   if op=='read':positions.append(offset);offset+=count
   else:positions.append(op)
  return {'positions':positions,'shared_offset':offset}
 if kind=='deadlines':return {'lateness':[max(0,t-d) for t,d in zip(x['dispatch'],x['deadlines'])],'expired_count':sum(d<=x['now'] for d in x['deadlines'])}
 if kind=='cycle':
  ns=set(x['edges']);ns.update(v for vs in x['edges'].values() for v in vs)
  def walk(n,path):return n in path or any(walk(v,path|{n}) for v in x['edges'].get(n,[]))
  return {'cyclic':any(walk(n,set()) for n in ns)}
 if kind=='ring':
  q=[];out=[];accepted=[]
  for op,val in x['operations']:
   if op=='push':ok=len(q)<x['capacity'];accepted.append(ok);q+=([val] if ok else [])
   else:out.append(q.pop(0) if q else None)
  return {'contents':q,'popped':out,'accepted':accepted}
 if kind=='quiescence':return {'eligible':sorted(v['id'] for v in x['retired'] if all(not p['active'] or p['epoch']>v['epoch'] for p in x['participants'])),'retained':sum(v['bytes'] for v in x['retired'] if not all(not p['active'] or p['epoch']>v['epoch'] for p in x['participants']))}
 if kind=='history':
  ops=x['operations'];witnesses=[]
  for order in itertools.permutations(range(len(ops))):
   if any(order.index(i)>order.index(j) for i in range(len(ops)) for j in range(len(ops)) if ops[i]['end']<ops[j]['start']):continue
   q=[];ok=True
   for i in order:
    op=ops[i]
    if op['op']=='push':q.append(op['value'])
    else:
     val=q.pop(0) if q else None
     if val!=op['value']:ok=False;break
   if ok:witnesses.append(order)
  return {'linearizable':bool(witnesses),'witness_count':len(witnesses)}
 if kind=='expression':
  # Fixture expressions only: independent arithmetic expression evaluator, not a target lexer/parser.
  assert re.fullmatch(r'[0-9+*() ]+',x['text'])
  return {'value':eval(x['text'],{'__builtins__':{}},{})}
 if kind=='encoding':
  assert x.get('mode',64)==64 and x.get('operand_bits',32)==32 and not x.get('prefixes',[]) and x['instruction'][0]==184 and len(x['instruction'])==5
  result={'immediate':int.from_bytes(bytes(x['instruction'][1:5]),'little'),'length':5,'opcode':x['instruction'][0]}
  if 'mode' in x:result.update(destination='EAX',addressing='register-immediate')
  return result
 raise ValueError('unknown oracle '+kind)

from pathlib import Path
import sys
OWNED={'T07','T08','T09','T10','T11','T13','T14','T15','T16','T23','T24','T25','T26'}
def metadata(path):
 match=re.search(r'\A```json\s*\n(.*?)\n```',path.read_text(encoding='utf-8'),re.S)
 if not match:raise ValueError('Missing leading canonical JSON metadata')
 return json.loads(match.group(1))
def verify(root):
 failures=[];bindings={};projects=0
 for module in sorted(OWNED):
  actual_projects=set()
  for path in sorted((root/'curriculum/projects').glob('PR-'+module+'-*.md')):
   try:
    m=metadata(path);pid=path.stem;projects+=1;actual_projects.add(pid)
    if m['id']!=pid or m['module']!=module:raise ValueError('Project ID/module/file mismatch')
    expected='fixtures/systems/'+pid+'.json'
    if m['fixtures']!=[expected]:raise ValueError('Canonical project fixture/file correspondence mismatch')
    if expected in bindings:raise ValueError('Shared/duplicate fixture ownership')
    bindings[expected]=pid+'-reference'
   except Exception as exc:failures.append(path.name+': '+repr(exc))
  try:
   m=metadata(root/'curriculum/modules'/ (module+'.md'))
   if m['id']!=module or len(m['projects'])!=len(set(m['projects'])) or set(m['projects'])!=actual_projects:raise ValueError('Module/project/file correspondence mismatch')
  except Exception as exc:failures.append(module+': '+repr(exc))
 files=sorted((root/'fixtures/systems').rglob('*.json'))
 actual_files={p.relative_to(root).as_posix() for p in files}
 for missing in sorted(set(bindings)-actual_files):failures.append('Missing canonical fixture: '+missing)
 for orphan in sorted(actual_files-set(bindings)):failures.append('Unowned/orphan fixture: '+orphan)
 seen_ids=set()
 for path in files:
  try:
   relative=path.relative_to(root).as_posix();f=json.loads(path.read_text(encoding='utf-8'))
   if f['id']!=bindings.get(relative):raise ValueError('Fixture ID/project/file correspondence mismatch')
   if f['id'] in seen_ids:raise ValueError('Duplicate fixture ID')
   seen_ids.add(f['id']);actual=oracle(f['oracle_type'],f['input'])
   if actual!=f['expected']:failures.append(path.name+': expected '+repr(f['expected'])+' got '+repr(actual))
  except Exception as exc:failures.append(path.name+': '+repr(exc))
 return projects,len(files),failures
def main():
 projects,count,failures=verify(Path(__file__).resolve().parents[1])
 print('systems fixtures:',count,'checked;',projects,'canonical projects;',len(failures),'failures')
 for failure in failures:print(failure)
 return 1 if failures or not count else 0
if __name__=='__main__':raise SystemExit(main())
