"""Independent finite ownership machine, including refused admission and quarantine."""
import argparse,hashlib,itertools,json,subprocess
def model(path,accepted,completed):
 state=0;abandoned=ticket=submits=waits=borrow=released=0;out=[]
 for op in path:
  if op=='S':
   if state:r=2
   else:
    submits+=1;r=0 if accepted else 3
    if accepted:state=1;abandoned=0;ticket=1;borrow=1
  elif op=='A':
   if not state:r=1
   else:abandoned=1;r={1:2,2:5,3:6}[state]
  elif op=='F':
   if not state:r=1
   elif state==3:r=6
   elif state==2:r=5 if abandoned else 4
   else:
    waits+=1
    if completed:state=2;borrow=0;r=5 if abandoned else 4
    else:state=3;r=6
  else:
   if state==2:released=1;state=abandoned=ticket=0;r=0
   else:r=6 if state==3 else 2
  out.append(','.join(map(str,[r,state,abandoned,ticket,submits,waits,borrow,released]))+';')
 return ''.join(out)
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 inputs=[];expected=[];transitions=0
 for accepted,completed in itertools.product(range(2),repeat=2):
  for n in range(1,7):
   for t in itertools.product('SAFR',repeat=n):
    path=''.join(t);inputs.append(f'{accepted} {completed} {path}\n');expected.append(model(path,accepted,completed));transitions+=n
 r=subprocess.run([a.driver,'oracle'],input=''.join(inputs),text=True,capture_output=True,timeout=90)
 assert r.returncode==0,r.stderr
 actual=r.stdout.splitlines();assert len(actual)==len(expected)
 for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(inputs[i],x,y)
 trace='\n'.join(expected)+'\n'
 print(json.dumps(dict(passed=True,histories=len(expected),transitions=transitions,trace_sha256=hashlib.sha256(trace.encode()).hexdigest())))
if __name__=='__main__':main()
