"""Wide independent expectations and complete selected ownership/clock traces."""
import itertools,json,math
MASK=2**64-1
def fields(line):
 def number(s):
  if ':' in s:
   high,low=map(int,s.split(':'));return high*2**32+low
  return int(s)
 return {k:number(v) for k,v in (s.split('=') for s in line.split()[1:] if '=' in s)}
def data(n):return [(i*7+3)%16 for i in range(n)]
def ordered_hash(values):
 value=0
 for x in values:value=(value*257+x+1)&MASK
 return value
def partition(n,p):
 sizes=[n//p+(i<n%p) for i in range(p)];ends=list(itertools.accumulate(sizes));return list(zip([0]+ends[:-1],ends))
def model():
 rows=[]
 for n in range(261):
  for p in range(1,5):
   spans=partition(n,p);indices=[i for lo,hi in spans for i in range(lo,hi)]
   if indices!=list(range(n)):raise ValueError('Partition ownership error')
   rows.append([n,p])
 # Equal sums can hide missing/duplicate identity even with nonnegative inputs.
 return {'passed':True,'partition_cases':len(rows),'fixture':{'ranges':[list(r) for r in partition(10,3)],'partials':[10,18,27],'sum':55},'dynamic257_sum':sum(i*i for i in range(1,258)),'checksum_counterexample':{'input':[1,1],'correct_indices':[0,1],'wrong_indices':[0,0],'both_sums':2},'floating_exact_integer_sum':2,'floating_expected':{'plain1':1,'plain2':0,'plain4':1,'compensated':2},'scope':'Mathematical partition, identity and arithmetic oracle; no physical performance theorem.'}
def check_differential(text):
 rows=[fields(s) for s in text.splitlines() if s.startswith('SR_DIFF ')];seen=set()
 for r in rows:
  n,p=r['n'],r['workers'];values=data(n);key=n,p
  if key in seen or n not in range(258) or p not in range(1,5):raise ValueError('Duplicate/unexpected differential case')
  seen.add(key)
  if r['sum']!=sum(i*i for i in range(1,n+1)) or r['hash']!=ordered_hash(values) or r['count']!=n or any(r['b'+str(i)]!=values.count(i) for i in range(16)):raise ValueError('Wrong wide independent result')
 if seen!={(n,p) for n in range(258) for p in range(1,5)}:raise ValueError('Missing differential cases')
 return {'cases':len(rows),'scope':'Actual C results compared with independent mathematical integers and sequential byte order.'}
def check_native(text):
 rows=[fields(s) for s in text.splitlines() if s.startswith('SR_NATIVE ')];places=[fields(s) for s in text.splitlines() if s.startswith('SR_PLACE ')];keys={(t,p,l) for t in range(3) for p in (1,2,4) for l in (0,1)}
 wanted_hash=ordered_hash(data(65539))
 if len(rows)!=18 or {(r['trial'],r['workers'],r['layout']) for r in rows}!=keys or 'SR_NATIVE_PASS:' not in text:raise ValueError('Missing equal-work native sample')
 for r in rows:
  if r['count']!=65539 or r['hash']!=wanted_hash or any(r[k]<=0 for k in ('wall_ns','cpu_ns','launch_ns','join_tail_ns','combine_ns','worker_cpu_ns')) or r['launch_ns']+r['join_tail_ns']+r['combine_ns']>r['wall_ns']:raise ValueError('Wrong native work/clock accounting')
  selected=[p for p in places if (p['trial'],p['workers'],p['layout'])==(r['trial'],r['workers'],r['layout'])]
  if len(selected)!=r['workers'] or {p['worker'] for p in selected}!=set(range(r['workers'])) or len({p['cpu'] for p in selected})!=r['workers']:raise ValueError('Workers did not use distinct eligible CPUs')
 launch=[fields(s) for s in text.splitlines() if s.startswith('SR_LAUNCH_FAIL ')]
 if launch!=[{'created':i,'joined':i,'visits':0,'unchanged':1} for i in range(4)]:raise ValueError('Wrong partial-launch cleanup')
 floats=[dict(word.split('=') for word in line.split()[1:]) for line in text.splitlines() if line.startswith('SR_FLOAT ')];floatkeys={(p,t,c) for p in (1,2,4) for t in range(3) for c in (0,1)}
 if len(floats)!=18 or {(int(r['workers']),int(r['repeat']),int(r['compensated'])) for r in floats}!=floatkeys:raise ValueError('Missing floating partition/repeat')
 for r in floats:
  p,c=int(r['workers']),int(r['compensated']);actual=float.fromhex(r['value']);expected=2 if c else 0 if p==2 else 1
  if actual!=expected or r['exact']!='2':raise ValueError('Wrong floating cancellation observation')
 return {'samples':rows,'placements':places,'launch_failures':launch,'floating':floats,'scope':'Native process clocks, eligible CPU placement and packed/padded atomic histogram rows. No physical coherence attribution or speedup claim; binary64 fixture accuracy is separate from reproducibility.'}
def check_guest(text,cpus):
 cases=[fields(s) for s in text.splitlines() if s.startswith('SR_CASE ')];parts=[fields(s) for s in text.splitlines() if s.startswith('SR_PART ')];hist=[fields(s) for s in text.splitlines() if s.startswith('SR_HIST')];final=[fields(s) for s in text.splitlines() if s.startswith('SR_GUEST_PASS:')]
 if final!=[{'cpus':cpus,'cases':6}] or len(cases)!=6 or len(parts)!=6*cpus:raise ValueError('Wrong guest shape')
 for sequence,n,mode,failure in ((1,0,0,0),(2,1,0,0),(3,10,0,0),(4,257,0,0),(5,257,0,1),(6,257,2,0)):
  selected=[r for r in cases if r['sequence']==sequence]
  if len(selected)!=1:raise ValueError('Missing case')
  r=selected[0];expected=91 if failure else ordered_hash(data(n)) if mode==2 else sum(i*i for i in range(1,n+1))
  if r['count']!=n or r['mode']!=mode or r['failed']!=failure or r['total']!=expected or r['sender_ticks']<=0:raise ValueError('Wrong guest total/failure/clock')
  workers=[r for r in parts if r['sequence']==sequence];ranges=partition(n,cpus)
  if {r['worker'] for r in workers}!=set(range(cpus)) or len({r['apic'] for r in workers})!=cpus:raise ValueError('Wrong worker/APIC coverage')
  for row in workers:
   i=row['worker'];lo,hi=ranges[i];failed=failure and i+1==cpus
   if row['cpu']!=i or (row['begin'],row['end'])!=(lo,hi) or row['completed']!=(0 if failed else hi-lo) or row['status']!=(4 if failed else 0) or row['active_ticks']<=0:raise ValueError('Wrong per-index partition or actual execution identity')
 if hist!=[{f'b{i}':data(257).count(i) for i in range(16)}]:raise ValueError('Wrong actual histogram')
 return {'cases':cases,'parts':parts,'histogram':hist,'scope':'Actual original virtual CPU execution and local virtual TSC only; no physical speedup.'}
if __name__=='__main__':print(json.dumps(model(),sort_keys=True))
