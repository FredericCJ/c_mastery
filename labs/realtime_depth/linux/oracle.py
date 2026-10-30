"""Independent integer selector/parameter oracle and exact guest-trace assessment."""
from itertools import product
import math
POLICIES={'other':0,'fifo':1,'rr':2,'deadline':6}
def inputs():
 rows=[]
 for lag in product((-1,0,1),repeat=3):
  for vd in product((0,1,2**64-1),repeat=3):
   for q,d,p in ((3,5,10),(3,2,10),(0,5,10),(3,5,4)):
    rows.append(' '.join(map(str,(lag[0],vd[0],lag[1],vd[1],lag[2],vd[2],q,d,p))))
 return rows
def check_model(text):
 expected=[]
 for row in inputs():
  a=list(map(int,row.split()));eligible=[i for i in range(3) if a[2*i]>=0];selected=min(eligible,key=lambda i:(a[2*i+1],i)) if eligible else 77
  expected.append(f'M {int(bool(eligible))} {selected} {int(0<a[6]<=a[7]<=a[8])}')
 if text.splitlines()!=expected:raise ValueError('Independent eligibility/parameter result mismatch')
 return {'rows':len(expected),'ineligible_snapshots':sum(line.split()[1]=='0' for line in expected)}
def fields(line):
 return {k:int(v) if v.lstrip('-').isdigit() else v for k,v in (part.split('=',1) for part in line.split()[1:])}
def checksum():
 state=1
 for _ in range(50000):state=(state*6364136223846793005+1)%(2**64)
 return state
def check_case(text,case,cpus,negative=False):
 mode,affinity,interference,behavior=case;lines=text.splitlines()
 if negative:
  if not any('LC_GUEST_CHECK:' in s for s in lines):raise ValueError('No intended guest semantic rejection')
  return {'semantic_rejection':True}
 if behavior=='invalid':
  if lines!=['LC_INVALID_REFUSED:Q3-D2-P10-before-policy-call']:raise ValueError('Invalid parameter did not fail before execution')
  return {'invalid_before_policy':True}
 cleanup=[fields(s) for s in lines if s.startswith('LC_CLEANUP ')];background=[fields(s) for s in lines if s.startswith('LC_BACKGROUND ')]
 if cleanup!=[{'background':int(interference!='none'),'reaped':1}]:raise ValueError('No exact owned interference cleanup witness')
 if interference=='none':
  if background:raise ValueError('Unexpected background process')
 elif background!=[{'kind':interference,'policy':3 if interference=='batch' else 0,'mask':1<<(cpus-1)}]:raise ValueError('Wrong actual interference class or placement')
 metadata=[fields(s) for s in lines if s.startswith('LC_META ')];ends=[fields(s) for s in lines if s.startswith('LC_END ')];jobs=[fields(s) for s in lines if s.startswith('LC_JOB ')]
 if len(metadata)!=1 or len(ends)!=1:raise ValueError('Missing/duplicate metadata')
 m=metadata[0];end=ends[0];available=(1<<cpus)-1;mask=1<<(cpus-1) if affinity=='one' else available
 if m['requested']!=mode or m['available']!=available or m['mask']!=mask or m['setup'] or end['reaped']!=1 or end['elapsed']<=0:raise ValueError('Environment/readback/cleanup mismatch')
 refused=behavior=='denied' or mode=='deadline' and cpus==2 and affinity=='one'
 if refused:
  if m['errno']!=1 or m['actual']!=0 or jobs or end['jobs'] or end['supervisor_kill'] or end['status']!=77*256 or 'LC_UNAVAILABLE:recorded-admission-refusal' not in lines:raise ValueError('Expected admission refusal was misclassified')
  return {'available':False,'errno':m['errno'],'actual_policy':m['actual'],'reason':'deliberate privilege or root-domain affinity restriction'}
 if m['errno'] or m['actual']!=POLICIES[mode]:raise ValueError('Requested class was not actually active')
 if mode=='deadline' and (m['runtime'],m['deadline'],m['period'],m['flags'])!=(3000000,5000000,10000000,5):raise ValueError('Wrong reservation readback')
 if mode=='rr' and m['quantum']<=0:raise ValueError('Missing actual RR quantum')
 if behavior=='infinite':
  if jobs or end['jobs'] or end['supervisor_kill']!=1 or end['status']!=9 or not 250000000<=end['elapsed']<2000000000 or 'LC_SUPERVISOR_PASS:owned-worker-terminated-and-reaped' not in lines:raise ValueError('No bounded supervisor termination witness')
  return {'available':True,'supervisor_elapsed_ns':end['elapsed'],'terminated_and_reaped':True}
 count=4 if behavior=='overrun' else 20
 if len(jobs)!=count or end['jobs']!=count or end['status'] or end['supervisor_kill'] or 'LC_GUEST_PASS:actual-policy-readback-workload' not in lines:raise ValueError('Incomplete measured workload')
 if behavior=='overrun' and (end['overruns']<1 or mode!='deadline'):raise ValueError('No actual reservation-overrun observation')
 for i,j in enumerate(jobs):
  if j['index']!=i or j['release']!=jobs[0]['release']+i*10000000 or not j['release']<=j['start']<=j['finish'] or j['cpu']<=0 or j['missed']!=int(j['finish']>j['release']+5000000):raise ValueError('Invalid original release/finish/CPU/miss trace')
  if behavior!='overrun' and j['checksum']!=checksum():raise ValueError('Wrong equivalent-work checksum')
  if behavior=='overrun' and j['cpu']<8000000:raise ValueError('Overrun stress did not consume declared CPU work')
 response=sorted(j['finish']-j['release'] for j in jobs)
 return {'available':True,'jobs':count,'application_misses':sum(j['missed'] for j in jobs),'overrun_signals':end['overruns'],'response_ns':{'minimum':response[0],'median':response[(count-1)//2],'p95':response[math.ceil(count*.95)-1],'maximum':response[-1]},'physical_deadline_guarantee':False}
def check_guest(text,cases,cpus,negative=False):
 lines=text.splitlines();summaries=[]
 for index,case in enumerate(cases):
  start='LC_CASE_BEGIN '+str(index)+' '+' '.join(case);matches=[i for i,s in enumerate(lines) if s==start]
  if len(matches)!=1:raise ValueError('Missing/duplicate case opening')
  begin=matches[0]+1;end=next((i for i in range(begin,len(lines)) if lines[i].startswith('LC_CASE_END '+str(index)+' status=')),None)
  if end is None:raise ValueError('Missing bounded case completion')
  status=int(lines[end].rsplit('=',1)[1]);summary=check_case('\n'.join(lines[begin:end]),case,cpus,negative)
  expected=1 if negative else 77 if summary.get('available') is False else 0
  if status!=expected:raise ValueError('Wrong guest process disposition')
  summaries.append({'case':case,'exit_code':status,**summary})
 if lines.count('LC_GUEST_DONE')!=1:raise ValueError('Guest did not finish its suite')
 if any(t in text for t in ('Kernel panic','BUG: KASAN','general protection fault')):raise ValueError('Kernel fault is never a semantic result')
 if 'LC_GUEST_CHECK:' in text and not negative:raise ValueError('Semantic check in accepted guest')
 return summaries
