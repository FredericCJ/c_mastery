"""Checks actual profiler, trace and paired observations. No performance winner assumed."""
import json,re,statistics,math
from fractions import Fraction
def pairs(text):
 rows=[json.loads(x) for x in text.splitlines()];out={}
 for n in (16,1024,65536):
  r=[x for x in rows if x['case']=='pair' and x['n']==n];assert len(r)==31 and [x['pair'] for x in r]==list(range(31)) and all(x['first']==x['pair']%2 and x['queries']==8192 for x in r)
  a=[x['a_ns'] for x in r];b=[x['b_ns'] for x in r];d=[x-y for x,y in zip(a,b)];assert all(x>0 for x in a+b)
  deadline=sorted(a)[math.ceil(.95*len(a))-1];out[str(n)]={'n':31,'median_A_ns':statistics.median(a),'median_B_ns':statistics.median(b),'median_paired_difference_ns':statistics.median(d),'difference_of_medians_ns':statistics.median(a)-statistics.median(b),'nearest_rank_p95_A_ns':deadline,'strict_misses_at_that_observed_threshold':sum(x>deadline for x in a),'maximum_A_ns':max(a),'threshold_policy':'posthoc descriptive,notpredeclared real-time requirement','raw_differences':d}
 bracket=[x['ns'] for x in rows if x['case']=='bracket'];assert len(bracket)==31;out['bracket_median_ns']=statistics.median(bracket);out['max_is_WCET']=False
 r=[x for x in rows if x['case']=='pareto'];assert len(r)==31 and all(x['binary_retained']==262144 and x['table_retained']==786436 for x in r);out['pareto']={'binary_median_ns':statistics.median(x['binary_ns'] for x in r),'table_median_ns':statistics.median(x['table_ns'] for x in r),'retained_bytes':[262144,786436],'timing_order':'AthenB;exploratory;notcausal randomizedclaim','correctness':'all8192queries/trial equal immutable-domain result','failure_premises':'live sortedextent and successful retainedlookup preparation before bracket'}
 branches=[x for x in rows if x['case']=='branch-pattern'];assert len(branches)==2 and all(x['comparisons']==1000000 and x['branch_misses'] is None for x in branches);out['branch_variants']=branches;return out
def trace(text):
 r=json.loads(next(x[len('PF_TRACE:'):] for x in text.splitlines() if x.startswith('PF_TRACE:')));assert r and len(r)<256 and r[-1]['asm'].startswith('ret');counts={'loads':0,'branches':0,'other':0}
 for x in r:
  op=x['asm'].split()[0]
  if op.startswith('j') or op.startswith('ret'):counts['branches']+=1
  elif '(' in x['asm']:counts['loads']+=1 # retained lower's only memory operation is cmp load;lea excluded below
  else:counts['other']+=1
  if op.startswith('lea'):counts['loads']-=1;counts['other']+=1
 counts['instructions']=len(r);counts['weighted_model_cycles']=counts['loads']*2+counts['branches']*3+counts['other'];counts['actual_cycles']=None
 counts['result']=int(re.findall(r'result=(\d+)',text)[-1]);counts['semantic_comparisons']=int(re.findall(r'comparisons=(\d+)',text)[-1]);assert counts['semantic_comparisons']==counts['loads'];counts['relative_pc_path']=[x['pc']-r[0]['pc'] for x in r];return counts
def symbols(text):
 result={}
 for s in text.splitlines():
  v=s.split()
  if len(v)>=4 and v[2].lower()=='t':result[v[3]]={'address':int(v[0],16),'bytes':int(v[1],16)}
 return result
def hooks(text,nm):
 syms=symbols(nm);names={v['address']:k for k,v in syms.items()};header=re.search(r'HOOK_BEGIN:(\d+):(\d+)',text);assert header
 n,lost=map(int,header.groups());events=[]
 for s in text.splitlines():
  if s.startswith('H:'):
   _,e,fn,caller,t=s.split(':');events.append((int(e),int(fn,16),int(t)))
 assert len(events)==n;stack=[];totals={};complete=not lost
 for enter,fn,t in events:
  if enter:stack.append([fn,t,0])
  elif not stack or stack[-1][0]!=fn:complete=False;break
  else:
   _,start,child=stack.pop();d=t-start;assert d>=child;row=totals.setdefault(names.get(fn,hex(fn)),{'calls':0,'inclusive_ns':0,'exclusive_ns':0});row['calls']+=1;row['inclusive_ns']+=d;row['exclusive_ns']+=d-child
   if stack:stack[-1][2]+=d
 if stack:complete=False
 if complete:assert totals['algo_lower']['calls']==1000 and 'pf_helper' not in totals
 return {'events':n,'dropped':lost,'complete':complete,'totals':totals if complete else None,'uninstrumented_helper_boundary_absent':all(fn!=syms['pf_helper']['address'] for _,fn,_ in events)}
def callgraph(text):
 # Ordinary direct calls from the ACTUAL linked disassembly; indirect edges
 # and removed/inlined symbols remain distinct in LTO outputs.
 current=None;edges=[];indirect=[]
 for line in text.splitlines():
  m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
  if m:current=m[1]
  if current and re.search(r'\bcall\w*\s',line):
   m=re.search(r'<([^>]+)>',line)
   (edges if m else indirect).append([current,m[1]] if m else current)
 return {'direct_edges':edges,'indirect_callers':indirect,'bounded_whole_program_stack':None}
def lower_cfg(text):
 ins=[]
 for line in text.splitlines():
  parts=line.split('\t')
  if len(parts)>=3 and re.match(r'\s*[0-9a-f]+:',parts[0]):ins.append((int(parts[0].strip().rstrip(':'),16),parts[-1].strip()))
 assert ins and ins[-1][1]=='ret';leaders={ins[0][0]};targets={}
 for i,(pc,s) in enumerate(ins):
  op=s.split()[0]
  if op.startswith('j'):
   target=int(s.split()[1],16);targets[pc]=target;leaders.add(target)
   if i+1<len(ins):leaders.add(ins[i+1][0])
 leaders=sorted(leaders);blocks={l:[] for l in leaders}
 for pc,s in ins:blocks[max(l for l in leaders if l<=pc)].append((pc,s))
 edges=set()
 for i,l in enumerate(leaders):
  pc,s=blocks[l][-1];op=s.split()[0]
  if op.startswith('j'):edges.add((l,targets[pc]))
  if op!='jmp' and op!='ret' and i+1<len(leaders):edges.add((l,leaders[i+1]))
 seen={leaders[0]}
 while True:
  new=seen|{b for a,b in edges if a in seen}
  if new==seen:break
  seen=new
 assert len(seen)==len(leaders)
 return {'blocks':len(leaders),'edges':len(edges),'metric':len(edges)-len(leaders)+2,'static_instructions':len(ins),'relative_edges':sorted((a-leaders[0],b-leaders[0]) for a,b in edges),'connected':True}
