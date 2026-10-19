"""Tri-state reducer that re-executes the actual bounded admission child."""
import argparse,json,subprocess
p=argparse.ArgumentParser();p.add_argument('binary');p.add_argument('events',nargs='?',default='NNANNAN');a=p.parse_args()
if len(a.events)>64:raise SystemExit('input capacity 64')
attempts=0
def classify(events):
 global attempts
 attempts+=1
 if attempts>256:raise SystemExit('replay budget exhausted')
 try:r=subprocess.run([a.binary,events],capture_output=True,text=True,timeout=2)
 except subprocess.TimeoutExpired:return 'UNRESOLVED'
 if r.returncode==0:return 'PASS'
 if r.returncode==1 and 'violations=' in r.stdout:return 'FAIL'
 return 'UNRESOLVED'
if classify(a.events)!='FAIL':raise SystemExit('initial input does not reproduce the same failure')
current=a.events;changed=True
while changed:
 changed=False
 for i in range(len(current)):
  candidate=current[:i]+current[i+1:]
  if classify(candidate)=='FAIL':current=candidate;changed=True;break
neighbors=[classify(current[:i]+current[i+1:]) for i in range(len(current))]
assert all(x!='FAIL' for x in neighbors)
print(json.dumps({'initial':a.events,'reduced':current,'deletion_neighbors':neighbors,'replays':attempts,'claim':'1-minimal deletion trigger for this same-failure predicate; not globally smallest or causal proof'}))
