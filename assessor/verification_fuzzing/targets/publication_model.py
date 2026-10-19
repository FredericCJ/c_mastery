"""Exhaustive reads-from graph check for this finite C11 atomic litmus only.

Not a general C11 model checker: no fences, consume, SC total order, non-atomic
payload, arbitrary RMWs, or out-of-thin-air cycles are represented.
"""
import itertools,json

def closure(edges):
 result=set(edges)
 while True:
  added={(a,d) for a,b in result for c,d in result if b==c}-result
  if not added:return result
  result|=added

def executions(mode,omit_sync=False):
 # Payload modification order I_p < P; flag I_f < F [< M for release sequence].
 final='M' if mode=='rmw-release-sequence' else 'S' if mode=='submission-only' else 'F'
 nodes={'I_p','I_f','S','P','F','R','Q'}|({'M'} if final=='M' else set())
 sb={('S','P'),('P','F'),('R','Q')}|({('F','M')} if final=='M' else set())
 mo={('I_p','P'),('I_f','S' if mode=='submission-only' else 'F')}|({('F','M'),('I_f','M')} if final=='M' else set())
 results=[]
 for source_flag,source_payload in itertools.product(['I_f',final],['I_p','P']):
  # Initialization before both threads, producer/consumer sequenced-before,
  # then the actual synchronizes-with edge justified by reads-from.
  hb=set(sb)|{(i,n) for i in ['I_p','I_f'] for n in nodes if n not in {'I_p','I_f'}}
  sw=set()
  if mode in {'release-acquire','rmw-release-sequence'} and source_flag==final and not omit_sync:sw.add(('F','R'))
  hb=closure(hb|sw)
  rf={('R',source_flag),('Q',source_payload)}
  # N1570 5.1.2.4 p19 write-read coherence: a write that happens before a
  # read cannot be bypassed by reading an earlier modification of that object.
  bad=[(write,read,source) for read,source in rf for write in nodes if (write,read) in hb and (source,write) in mo]
  results.append(dict(mode=mode,reads_from=dict(rf),synchronizes_with=sorted(sw),happens_before=sorted(hb),coherence_violations=bad,allowed=not bad,observed=[int(source_flag!= 'I_f'),19 if source_payload=='P' else 0]))
 return results

def main():
 rows=[]
 for mode,expected in [('relaxed',4),('release-acquire',3),('rmw-release-sequence',3),('submission-only',4)]:
  group=executions(mode);rows+=group;assert sum(r['allowed'] for r in group)==expected
  forbidden=next(r for r in group if r['observed']==[1,0]);assert forbidden['allowed']==(mode in {'relaxed','submission-only'})
 # Named semantic negative control: dropping the SW edge admits stale payload
 # even though the observation reads the producer's release (or its RMW sequence).
 mutation=executions('release-acquire',omit_sync=True)
 assert sum(r['allowed'] for r in mutation)==4
 assert any(r['allowed'] and r['observed']==[1,0] for r in mutation)
 print(json.dumps(dict(passed=True,scope='Four finite two-actor atomic litmus variants; exhaustive4 reads-from choices each',executions=rows,negative_control='Omitting synchronizes-with admits stale payload',limitations=['Graph rules are specialized to this program and inspected coherence/synchronization passages','Not general ISO C11 consistency validation, hardware observation, fairness or liveness proof','All payload accesses atomic; replacing with non-atomic storage requires a separate data-race analysis'])))
if __name__=='__main__':main()
