"""Independent finite publication, copied-version and close/drain state model."""
import argparse,hashlib,itertools,json,subprocess
def check(driver):
 inputs=[];expected=[]
 for history in itertools.product(range(6),repeat=3):
  current=0;closed=False;slots=[None,None]
  for i,op in enumerate(history):
   inputs.append(f'{int(i==0)} {op}');status=0
   if op<2:
    if closed:status=22
    else:current=op+1
   elif op==2:
    if closed:status=22
    elif None not in slots:status=3
    else:slots[slots.index(None)]=current
   elif op==3:
    active=next((j for j,x in enumerate(slots) if x is not None),None)
    if active is None:status=1 # driver no-owned-lease observation, no invalid release call
    else:slots[active]=None
   elif op==4:closed=True;status=20 if any(x is not None for x in slots) else 0
   else:status=22 if closed else 0
   expected.append(' '.join(map(str,[status,-22 if closed else current]+[-1 if x is None else x for x in slots])))
 p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr
 rows=p.stdout.splitlines();assert rows==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(rows,expected)) if a!=b),('length',len(rows),len(expected)))
 return dict(passed=True,histories=216,transitions=648,trace_sha256=hashlib.sha256(p.stdout.encode()).hexdigest(),limits='Serialized finite copied-version model; native competing threads tested separately, not an exhaustive interleaving or C memory-model proof.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
