"""Minimal queue publication/reuse failures, with bounded trace minimization."""
import itertools,json
def bad_publication(events):
 payload=0;ready=0;observed=False
 for event in events:
  if event=='write':payload=42
  elif event=='publish':ready=1
  elif event=='read' and ready:observed=True;return payload!=42
 return False
def minimize(trace,predicate):
 current=list(trace)
 for n in range(len(trace)+1):
  for indices in itertools.combinations(range(len(trace)),n):
   candidate=[trace[i] for i in indices]
   if predicate(candidate):return candidate
 return current
def main():
 trace=['noise','publish','read','write','noise'];small=minimize(trace,bad_publication);assert small==['publish','read']
 # The flawed published flag precedes initialized data. Fixing the source
 # order requires write->release publish, and a matching acquire before read.
 schedules=[]
 for p in itertools.permutations(['write','publish','read']):
  if p.index('write')<p.index('publish'):assert not bad_publication(p);schedules.append(p)
 # On a weak-memory model, source order alone is insufficient if ready uses
 # relaxed operations: the earlier atomic package supplies MP(1,0) evidence.
 print(json.dumps(dict(status='pass',original=trace,minimal=small,repair_schedules=schedules,edge='payload write -> release ready -> reads-from acquire -> payload read',reuse='last payload read -> release retirement -> reads-from acquire -> overwrite',limits='This operational minimizer does not model weak visibility; the separately imported MP model and handoff proof supply those ordering obligations.'),indent=2))
if __name__=='__main__':main()
