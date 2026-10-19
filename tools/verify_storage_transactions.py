"""Independent finite map/savepoint model of the wrapper's state transitions."""
import argparse,hashlib,itertools,json,subprocess
def check(driver):
 lines=[];expected=[];histories=0
 for trace in itertools.product(range(1,10),repeat=3):
  histories+=1;active=True;saved=False;serial=gen=current=committed=undo=0
  for step,op in enumerate(trace):
   lines.append(f'{int(step==0)} {op}');status=0
   if op==8:
    if active or saved:status=6
    else:active=True;current=committed
   elif not active:status=6
   elif op in [1,2]:current|=1<<(op-1)
   elif op==3:
    if not current&1:status=1
    else:current&=~1
   elif op==4:
    if saved:status=6
    else:undo=current;saved=True;serial+=1
   elif op in [5,6]:
    if not saved:status=6
    else:
     if op==5:current=undo
     saved=False
   elif op==7:active=False;saved=False;current=committed
   elif op==9:committed=current;gen+=1;active=False;saved=False
   expected.append(f'{status} {int(active)} {int(saved)} {serial} {gen} {current}')
 p=subprocess.run([driver,'oracle'],input='\n'.join(lines)+'\n',text=True,capture_output=True,check=True);rows=p.stdout.splitlines();assert rows==expected,next(((i,a,b,lines[i]) for i,(a,b) in enumerate(zip(rows,expected)) if a!=b),('length',len(rows),len(expected)))
 return dict(passed=True,histories=histories,transitions=len(lines),trace_sha256=hashlib.sha256(p.stdout.encode()).hexdigest(),model='Two-key map, one savepoint, initially active transaction; all three-operation words over nine actual wrapper operations. Split failures and finite serial boundary are separate anchors.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
