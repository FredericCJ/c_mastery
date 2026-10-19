"""Independent finite lease histories and observed emulator startup invariants."""
import itertools,re
def cases():
 rows={(p,s,''.join(w)) for p in (0,1) for s in (0,2**32-2) for n in range(5) for w in itertools.product('BTASPQR',repeat=n)}
 rows|={(1,0,''.join(w)) for n in range(5) for w in itertools.product('BCADPKRJ',repeat=n)}
 return sorted(rows)
def inputs():return [f'{p} {s} {w or "-"}' for p,s,w in cases()]
def expected(parallel,seed,word):
 phases=[0,0];generations=[seed,0];tickets=[0,0];results=[]
 for event in word:
  slot=1 if event in 'CDKJ' else 0
  if event in 'BC':
   if phases[slot] or not parallel and any(phases):result=2
   elif generations[slot]==2**32-1:result=5
   else:generations[slot]+=1;phases[slot]=1;tickets[slot]=generations[slot];result=0
  else:
   generation=max(0,tickets[slot]-1) if event=='S' else tickets[slot]
   if not phases[slot] or generation!=generations[slot]:result=3
   elif event=='T':
    if phases[slot]!=1:result=2
    else:phases[slot]=2;result=0
   elif event in 'ADS':
    if phases[slot] in (3,4,5):result=4
    else:result=6 if phases[slot]==2 else 0;phases[slot]=4 if result==6 else 3
   elif event=='Q':result=7
   elif event in 'PK':
    if phases[slot] not in (3,4):result=2
    else:phases[slot]=5;result=0
   elif event in 'RJ':
    if phases[slot]!=5:result=7
    else:phases[slot]=0;result=0
   else:raise ValueError('unknown event')
  results.append(result)
 return 'SS'+''.join(' '+str(r) for r in results)+' | '+str(phases[0])+' '+str(generations[0])+' '+str(phases[1])+' '+str(generations[1])
def check_model(text):
 rows=cases();lines=text.splitlines()
 if len(lines)!=len(rows):raise ValueError('Missing/extra model history')
 for row,line in zip(rows,lines):
  if line!=expected(*row):raise ValueError('Wrong lease history '+str(row))
 return {'histories':len(rows),'maximum_depth':4,'scope':'Exhaustive over declared short word sets/initial counters, not all executions or port quiescence proof.'}
def check_guest(text,cpus,base,mode,negative=False):
 if any(token in text for token in ('FAIL SMP:','Kernel panic','Segmentation fault','HARNESS_TIMEOUT')):raise ValueError('Fault/timeout is not semantic evidence')
 if negative:
  if 'SS_GUEST_CHECK:' not in text or 'SS_GUEST_PASS:' in text:raise ValueError('Missing intended startup rejection')
  return {'intended_rejection':True}
 if text.count('SS_GUEST_PASS:')!=1 or 'SS_GUEST_CHECK:' in text:raise ValueError('Missing verified completion')
 bsp=re.findall(r'SS_BSP cpus=(\d+) apic=(\d+) mode=(\d+) base=(\d+)',text)
 if bsp!=[(str(cpus),'0',str(mode),str(base))]:raise ValueError('Wrong BSP/model configuration')
 fields=re.findall(r'SS_CPU index=(\d+) apic=(\d+) generation=(\d+) verified=(\d+) page=(\d+):(\d+) bootstrap_entry=(\d+):(\d+) permanent_top=(\d+):(\d+) late=(\d+)',text)
 if len(fields)!=cpus-1:raise ValueError('Missing/duplicate AP completion')
 rows=[];expected_ids=[0,1,2,4] if cpus==4 else list(range(cpus));boots=set();permanent=set()
 for index,fields in enumerate(fields,1):
  c,apic,generation,verified,ph,pl,bh,bl,kh,kl,late=map(int,fields);page=(ph<<32)|pl;boot=(bh<<32)|bl;park=(kh<<32)|kl
  if (c,apic,generation,verified,page,late)!=(index,expected_ids[index],1 if mode==1 else index,15,base+(index-1)*4096 if mode==1 else base,int(mode==2 and index==1)):raise ValueError('Wrong sparse identity/generation/verified state')
  if (boot+8)%16 or park%16 or boot==park or boot in boots or park in permanent:raise ValueError('Wrong private/bootstrap/permanent stack identity')
  boots.add(boot);permanent.add(park);rows.append({'index':c,'apic':apic,'generation':generation,'page':page,'bootstrap_entry':boot,'permanent_top':park,'late':late})
 timeouts=re.findall(r'SS_TIMEOUT_RETAIN:index=(\d+) generation=(\d+)',text)
 if timeouts!=([('1','1')] if mode==2 and cpus>1 else []):raise ValueError('Missing/extra timeout-retention experiment')
 return rows
